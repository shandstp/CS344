#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg){ perror(msg); exit(1); } //Error function for reportin issues

int ctoi(char ch){
	const char* legend = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i = 0;
	while(i < 27 && legend[i] != ch){
		i++;
	}
	return i;
}

char itoc(int ind){
	const char* legend = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	return legend[ind];
}

void growBuff(char** bf, int* sz){
	char* tmp = malloc(sizeof(char) * (*sz));
	strcpy(tmp, *bf);
	free(*bf);	
	*sz *= 2;
	*bf = malloc(sizeof(char) * (*sz));
	memset(*bf, '\0', (*sz));
	strcpy(*bf, tmp);
	free(tmp);
}

void kodzukuri(int fd){
	pid_t cpid = fork();
	switch(cpid){
		case -1:{
				perror("There was a problem creating child");
				exit(1);
				break;
			}
		case 0:{
			       int charsRead = 0; 
			       int bufSize = 8;
			       char** buffer = malloc(sizeof(char*));
			       *buffer = malloc(sizeof(char) * bufSize);

			       //Get the message from the client and display it
			       memset(*buffer, '\0', bufSize);
			       char chunk[5];

			       //read chunks of 3 bytes until entire message and key are read
			       do{
				       memset(chunk, '\0', 5);
				       charsRead += recv(fd, chunk, 4, 0); //Read the client's message from the socket
				       if(charsRead < 0) error("ERROR reading from socket");
				       if((charsRead + 5) >= bufSize){ growBuff(buffer, &bufSize); } 
				       strcat(*buffer, chunk); //accumulate chunks until entire message is read
			       } while(strstr(chunk, "@@") == NULL); //@@ indicates end of message
			       //		printf("SERVER: I received this from the client: \"%s\"\n", *buffer);

			       //process string read into key and message pair 
			       char* key = strstr(*buffer, "!!"); //set key to the next address after the message
			       key[0] = '\0'; //terminate message string
			       key += 2; //increment address of key such that it now points at the start of the key
			       int msgSize = strlen(*buffer);
			       char msg[msgSize];
			       memset(msg, '\0', msgSize);
			       strcpy(msg, *buffer);
			       memset(*buffer, '\0', msgSize);
			       int i = 0;
			       while(msg[i] != '\0'){
				       int msgNum = ctoi(msg[i]);
				       int keyNum = ctoi(key[i]);
				       int modNum = (msgNum - keyNum);
					if(modNum < 0){ modNum += 27; }
				       buffer[0][i] = itoc(modNum); 
				       i++;
			       }
			       int repSize = strlen(*buffer);
			       strcat(*buffer, "@@"); 
			       //Send a success message back to the client
			       charsRead = send(fd, *buffer, repSize + 2, 0); //Send success back
			       if(charsRead < 0) error("ERROR writing to socket");
			       close(fd); //Close the existing socket which is connected to the client
			       exit(0);
			       break;
		       }
		default: break;
	}
}

void bruceCampbell(){
	pid_t cpid = fork();
	while(cpid == 0){
		wait(NULL);
	}
}

int main(int argc, char *argv[]){
	int listenSocketFD, establishedConnectionFD, portNumber;
	int conCount = 0;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if(argc < 2) {fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } //Check usage & args
	//Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); //Clear out the address struct
	portNumber = atoi(argv[1]); //Get the port number, convert to an interger from a string
	serverAddress.sin_family = AF_INET; //Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); //Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; //Any address is allowed for connection to this process

	//Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); //Create the socket
	if(listenSocketFD < 0) error("ERROR opening socket");

	//Enable the socket to begin listening
	if(bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) //Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); //Flip the socket on - it can now receive up to 5 connections
	bruceCampbell();

	while(1){
		pid_t child = waitpid(-1, NULL, WNOHANG);
		if(child > 0){
			conCount--;
		}
		//Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); //Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); //Accept
		char clientID[4];
		memset(clientID, '\0', 4);	
		recv(establishedConnectionFD, clientID, 4, 0); //Read the client's id from the socket
		send(establishedConnectionFD, "dec\0", 4, 0); 
		if(strcmp(clientID, "dec") == 0){ 
			if(establishedConnectionFD < 0) error("ERROR on accept");
			if(conCount < 5){
				kodzukuri(establishedConnectionFD);
				conCount++;
			}
			else{
				wait(NULL);
				conCount--;
			}
		}
		else{ close(establishedConnectionFD); }
	}
	close(listenSocketFD); //Close the listening socket
	return 0;
}
