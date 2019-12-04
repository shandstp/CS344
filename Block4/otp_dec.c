#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define varName(name) #name

void error(const char *msg){perror(msg); exit(0);}

void validate(char* str){
	int len = strlen(str);
	for(int i = 0; i < len; i++){
		if(!(str[i] > 64 && str[i] < 91) && str[i] != 32){
			fprintf(stderr, "CLIENT: ERROR, key and message can only contain A-Z or space characters\n");
			exit(0);
		}
	}
}

int main(int argc, char *argv[]){
	if(argc < 4){fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]); exit(0);}

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	//Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //Clear out the address struct
	portNumber = atoi(argv[3]); //Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; //Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);
	serverHostInfo = gethostbyname("localhost"); //Convert the machine name into a special form of address
	if(serverHostInfo == NULL){ fprintf(stderr, "CLIENT: ERROR, unable to connect to local host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); //Copy in the address

	//Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); //Create the socket
	if(socket < 0) error("CLIENT: ERROR opening socket");

	//Connect to server
	if(connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) //Connect socket to address
		error("CLIENT: ERROR connection");

	//open key and message files for reading
	int msgFD = open(argv[1], O_RDONLY);
	if(msgFD < 0) error("CLIENT: ERROR opening message file");
	int keyFD = open(argv[2], O_RDONLY);
	if(keyFD < 0) error("CLIENT: ERROR opening key file");

	//read from message file 
	off_t msgLen = lseek(msgFD, 0, SEEK_END); //get size of string to be read in
	lseek(msgFD, 0, SEEK_SET); //set file pointer back to beggining 
	char* msg = malloc((sizeof(char) * msgLen)); //allocate enough space for entire string and terminating char
	memset(msg, '\0', sizeof(*msg)); //initialize new string with null terminator chars
	ssize_t bytesRd = read(msgFD, msg, msgLen); //read conents of message file
	if(bytesRd < msgLen){ fprintf(stderr, "CLIENT: ERROR, failed to read contents of message\n"); exit(0); }
	msg[msgLen - 1] = '\0'; //replace final \n with \0

	//read from key file
	off_t keyLen = lseek(keyFD, 0, SEEK_END); //get size of string to be read in
	lseek(keyFD, 0, SEEK_SET); //set file pointer back to beggining
	char* key = malloc((sizeof(char) * keyLen)); //allocate enough space for the entire string and terminating char
	memset(key, '\0', sizeof(*key)); //initialize new string with null terminator chars
	bytesRd = read(keyFD, key, keyLen); //read contents of message file
	if(bytesRd < keyLen){ fprintf(stderr, "CLIENT: ERROR, failed to read contents of key\n"); exit(0); }
	key[keyLen - 1] = '\0'; //replace final \n with \0

	validate(msg);
	validate(key);

	if(keyLen < msgLen){ fprintf(stderr, "CLIENT: ERROR, the key must be at least as long as the message\n"); exit(0); } //verify that message and key are the same length

	char* buffer = malloc(sizeof(char) * (msgLen + keyLen + 8));
	char clientID[4];
	char serverID[4];	
	memset(clientID, '\0', 4);
	memset(serverID, '\0', 4);
	strcpy(clientID, "dec");
	send(socketFD, clientID, 3, 0);
	recv(socketFD, serverID, 3, 0);
	if(strcmp(clientID, serverID) != 0){fprintf(stderr, "CLIENT: ERROR, attempted to connect to invalid server!\n"); exit(2); }	
	sprintf(buffer, "%s%s%s%s", msg, "!!", key, "@@");

	//Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); //Write to the server
	if(charsWritten < 0 ) error("CLIENT: ERROR writing to socket");
	if(charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	//Get return message from server
	memset(buffer, '\0', (msgLen + keyLen + 8)); //Clear out the buffer again for reuse
	char chunk[5];
	char* strEnd;
	memset(chunk, '\0', 5);
	do{
		charsRead = recv(socketFD, chunk, 4, 0); //Read data from the socket, leaving \0 at end
		if(charsRead < 0) error("CLIENT: ERROR reading from socket");
		strcat(buffer, chunk);
	} while((strEnd = strstr(buffer, "@@")) == NULL);
	memset(strEnd, '\0', 2);
//	buffer[strEnd - buffer] = '\0';
	printf("%s\n", buffer);

	close(socketFD); //Close the socket
	return 0;
}
