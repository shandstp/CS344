#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg){ perror(msg); exit(1); } //Error function for reportin issues

//takes a char or space and returns the corresponding integer value between 0 and 26
int ctoi(char ch){
	const char* legend = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i = 0;
	//loop through each character in the legend until a match is found
	while(i < 27 && legend[i] != ch){
		i++;
	}
	return i; //return integer value for char received
}

//takes integer and returns the corresponding char
char itoc(int ind){
	const char* legend = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	return legend[ind]; //returns the char located at the index matching the integer provided
}

//takes a point to string, allocates a string with double it's length, copies the contents of the 
//old string to the new one and updates it's size to relect it's new size
void growBuff(char** bf, int* sz){
	//didn't notice until just now that I could have done this all in fewer steps
	char* tmp = malloc(sizeof(char) * (*sz)); //allocate space to temporarily hold the string for some reason
	strcpy(tmp, *bf); //copy contents on buffer to tmp string in a superfluous step
	free(*bf); //free buffer	
	*sz *= 2; //double size of buffer
	*bf = malloc(sizeof(char) * (*sz)); //allocate space for buffer with new size
	memset(*bf, '\0', (*sz)); //initialize buffer
	strcpy(*bf, tmp); //copy contents of temp to new buffer
	free(tmp); //free temp
}

//spawn child process once connection has been established with a valid client
void kodzukuri(int fd){
	pid_t cpid = fork(); //spawn child
	switch(cpid){
		case -1:{
				perror("There was a problem creating child"); //report error and move on if there was a problem while spawning child
				break;
			}
		//if current process is child, move on to communicating with client
		case 0:{
			       int charsRead = 0; //track how many chars have been read so far
			       int bufSize = 8; //initial size of buffer
			       char** buffer = malloc(sizeof(char*)); //allocate space on heap for pointer to pointer
			       *buffer = malloc(sizeof(char) * bufSize); //allocate space on heap for buffer being pointed to

			       //Get the message from the client and display it
			       memset(*buffer, '\0', bufSize); //initialize buffer
			       char chunk[5]; //stores data received from client in 5 byte chunks

			       //read chunks of 3 bytes until entire message and key are read
			       do{
				       memset(chunk, '\0', 5); //initialize chunk
				       charsRead += recv(fd, chunk, 4, 0); //Read the client's message from the socket
				       if(charsRead < 0) error("ERROR reading from socket");
				       //check if buffer has enough room to store another chunk, grow if not
				       if((charsRead + 5) >= bufSize){ growBuff(buffer, &bufSize); }
				       strcat(*buffer, chunk); //accumulate chunks until entire message is read
			       } while(strstr(chunk, "@@") == NULL); //@@ indicates end of message

			       //process string read into key and message pair 
			       char* key = strstr(*buffer, "!!"); //set key to the next address after the message
			       key[0] = '\0'; //terminate message string
			       key += 2; //increment address of key such that it now points at the start of the key
			       int msgSize = strlen(*buffer); //get size of the plaintext message
			       char msg[msgSize]; //stores plaintext message
			       memset(msg, '\0', msgSize); //initialize plaintext message
			       strcpy(msg, *buffer); //store plaintext message in msg
			       memset(*buffer, '\0', msgSize); //prepare buffer to store the encoded message
			       int i = 0;
			       //increment through each character of the plaintext message, encode it, and store the encoded char into the appropriate index of buffer
			       while(msg[i] != '\0'){
				       int msgNum = ctoi(msg[i]); //get integer representation of current plaintext char
				       int keyNum = ctoi(key[i]); //get integer representation of current key char
				       int modNum = ((msgNum + keyNum) % 27); //calculate integer value of encoded char to be stored in buffer
				       buffer[0][i] = itoc(modNum);  //store encoded char in current index of buffer
				       i++;
			       }
			       int repSize = strlen(*buffer); //indicates size of reply to be sent to client
			       strcat(*buffer, "@@"); //appends @@ to indicate end of transmission
			       charsRead = send(fd, *buffer, repSize + 2, 0); //send encoded message back to client including terminating @@
			       if(charsRead < 0) error("ERROR writing to socket");
			       close(fd); //Close the existing socket which is connected to the client
			       exit(0); //child process has finished its work
			       break;
		       }
		default: break;
	}
}

//was an attempt to use a child process to continually clean up fellow child processes
//don't know if that's possible but wasn't able to get it to work
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
//	bruceCampbell(); 

	while(1){
		pid_t child = waitpid(-1, NULL, WNOHANG); //look for zombie children before establishing a new conneciton
		if(child > 0){
			conCount--; //if a child has exited decrement connection count
		}
		//Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); //Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); //Accept
		char clientID[4]; //stores id code of connected client
		memset(clientID, '\0', 4); //initialize clientID	
		recv(establishedConnectionFD, clientID, 4, 0); //Read the client's id from the socket
		send(establishedConnectionFD, "enc\0", 4, 0);  //send server id to client
		if(strcmp(clientID, "enc") == 0){  //if client shares same id as server, spawn child to handle communications
			if(establishedConnectionFD < 0) error("ERROR on accept");
			if(conCount < 5){ //if less than 5 clients are currently connected, communications can begin with a new one
				kodzukuri(establishedConnectionFD); //spawn child to handle communications with client
				conCount++; //increase connection count
			}
			else{
				wait(NULL); //if too many clients are connected wait till one exits and try again
				conCount--; //drecrement connection count
			}
		}
		else{ close(establishedConnectionFD); } //close connection if communication wasn't allowed
	}
	close(listenSocketFD); //Close the listening socket
	return 0;
}
