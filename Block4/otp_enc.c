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

void error(const char *msg){perror(msg); exit(0);} //simplify calling perror

//verify that the given string does not contain invalid characters(i.e. anything other than A-Z or space)
void validate(char* str){
	int len = strlen(str);
	for(int i = 0; i < len; i++){ //increment through each letter in the string
		if(!(str[i] > 64 && str[i] < 91) && str[i] != 32){ //if an invalid char is found throw an error and exit with status 1
			fprintf(stderr, "CLIENT: ERROR, key and message can only contain A-Z or space characters\n");
			exit(1);
		}
	}
}

int main(int argc, char *argv[]){
	//if the user does not provide at least four arguments educate them on how to use this program and exit with status 1
	if(argc < 4){fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]); exit(1);} 

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	//Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //Clear out the address struct
	portNumber = atoi(argv[3]); //Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; //Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); //translate port given by user into network format
	serverHostInfo = gethostbyname("localhost"); //Convert the machine name into a special form of address
	if(serverHostInfo == NULL){ fprintf(stderr, "CLIENT: ERROR, unable to connect to local host on port %s\n", argv[3]); exit(2); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); //Copy in the address

	//Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); //Create the socket
	if(socket < 0) error("CLIENT: ERROR opening socket");

	//Connect to server
	if(connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) //Connect socket to address
		error("CLIENT: ERROR connection");

	//open key and message files for reading
	int msgFD = open(argv[1], O_RDONLY); //open plaintext file indicated by user; throw error on failure
	if(msgFD < 0) error("CLIENT: ERROR opening message file");
	int keyFD = open(argv[2], O_RDONLY); //open key file indicated by user; throw error on failure
	if(keyFD < 0) error("CLIENT: ERROR opening key file");

	//read from message file 
	off_t msgLen = lseek(msgFD, 0, SEEK_END); //get size of string to be read in
	lseek(msgFD, 0, SEEK_SET); //set file pointer back to beggining 
	char* msg = malloc((sizeof(char) * msgLen)); //allocate enough space for entire string and terminating char
	memset(msg, '\0', sizeof(*msg)); //initialize new string with null terminator chars
	ssize_t bytesRd = read(msgFD, msg, msgLen); //read conents of message file
	if(bytesRd < msgLen){ fprintf(stderr, "CLIENT: ERROR, failed to read contents of message\n"); exit(1); }
	msg[msgLen - 1] = '\0'; //replace final \n with \0

	//read from key file
	off_t keyLen = lseek(keyFD, 0, SEEK_END); //get size of string to be read in
	lseek(keyFD, 0, SEEK_SET); //set file pointer back to beggining
	char* key = malloc((sizeof(char) * keyLen)); //allocate enough space for the entire string and terminating char
	memset(key, '\0', sizeof(*key)); //initialize new string with null terminator chars
	bytesRd = read(keyFD, key, keyLen); //read contents of message file
	if(bytesRd < keyLen){ fprintf(stderr, "CLIENT: ERROR, failed to read contents of key\n"); exit(1); }
	key[keyLen - 1] = '\0'; //replace final \n with \0

	//verify that the provided plaintext and key do no contain any invalid characters
	validate(msg);
	validate(key);
	
	//verify that the key is at least as long as the message to be encrypted 
	if(keyLen < msgLen){ fprintf(stderr, "CLIENT: ERROR, the key must be at least as long as the message\n"); exit(1); } 

	char* buffer = malloc(sizeof(char) * (msgLen + keyLen + 8)); //make sure buffer has enough room for plaintext, key, and special characters
	char clientID[4]; //stores three byte code to be sent to server to identify which client this is
	char serverID[4]; //store three byte code to be received from the server to identify which server it is	
	memset(clientID, '\0', 4); //initialize for use
	memset(serverID, '\0', 4); //initialize for use
	strcpy(clientID, "enc"); //prepare to send enc signal to server
	send(socketFD, clientID, 3, 0); //send enc identifier to server
	recv(socketFD, serverID, 3, 0); //receive either enc or dec from server
	//if server and client do not share the same identification code, throw and error and exit with status 2
	if(strcmp(clientID, serverID) != 0){fprintf(stderr, "CLIENT: ERROR, attempted to connect to invalid server on port %s\n", argv[3]); exit(2); }	
	sprintf(buffer, "%s%s%s%s", msg, "!!", key, "@@"); //if communicating with correct server prepare to send string of format msg!!key@@

	//Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); //Write to the server
	if(charsWritten < 0 ) error("CLIENT: ERROR writing to socket");
	if(charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n"); //verify entire message was sent

	//Get return message from server
	memset(buffer, '\0', (msgLen + keyLen + 8)); //Clear out the buffer again for reuse
	char chunk[5]; //store data received from server in 5 byte chunks
	char* strEnd; //stores address of terminating @@ string to determine when end of input has been reached
	memset(chunk, '\0', 5);
	do{
		charsRead = recv(socketFD, chunk, 4, 0); //Read data from the socket, leaving \0 at end
		if(charsRead < 0) error("CLIENT: ERROR reading from socket");
		strcat(buffer, chunk); //add current data chunck to string read in so far
	} while((strEnd = strstr(buffer, "@@")) == NULL); //exit loop once @@ has been found
	memset(strEnd, '\0', 2); //ensure string is null terminated by replacing @@ with '\0'
	//	buffer[strEnd - buffer] = '\0';
	printf("%s\n", buffer); //print string received from server

	close(socketFD); //Close the socket
	return 0;
}
