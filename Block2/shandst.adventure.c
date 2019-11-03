#define _GNU_SOURCE //Silences warning about implicit declaration of pthread_tryjoin_np

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t myMute = PTHREAD_MUTEX_INITIALIZER; //Initialize mutex

//Used to keep track of how many steps the user took, which rooms they visited, and in what order
struct gameState{
	int steps; //stores a count of how many steps a user took toward victory
	char** visited; //stores the name of each room visited during game play in the order in which the rooms were visited
};

struct roomFile{
	int numEntries; //Stores a count of how many entries were found in room's room file
	char** entries; //Stores name of current room at entries[0], the room's type at entries[numEntries], and names of connecting rooms at every index in between
};

//Opens ".", checks each directory that begins with "shandst.rooms." to find the most recently created, and returns its name
char* selectDir(){

	int mostRecent = -1; //prepare to store timestamp of most recent directory
	char dirPrefix[32] = "shandst.rooms."; //Only directories starting with this string should be checked
	int nameLen = 256;
	char* newDir = malloc(nameLen * sizeof(char)); //Prepare to store name of most recently created directory
	memset(newDir, '\0', nameLen);

	DIR* topDir;
	struct dirent *curDir;
	struct stat dirInfo;

	topDir = opendir("."); //Open the present working directory
	if(topDir > 0){
		while((curDir = readdir(topDir)) != NULL){ //Read each entry in the directory
			if(strstr(curDir->d_name, dirPrefix) != NULL){ //If currently examined directory contains substring "shands.rooms." get its timestamp
				stat(curDir->d_name, &dirInfo);
				
				if((int)dirInfo.st_mtime > mostRecent){ //Compare current timestamp to current mostRecent. Replace if more recent
					mostRecent = (int)dirInfo.st_mtime;
					memset(newDir, '\0', nameLen);
					strcpy(newDir, curDir->d_name);
				}
			}
		}
	}


	closedir(topDir);
	return newDir; //Return name of most recently created directory
}

//Sets roomFile to default values
void initFile(struct roomFile* rf){
	rf->numEntries = 0; //Sets numEntries to 0 so that it can be used to count how many values have been read in from the current room file
	rf->entries = malloc(8 * sizeof(char*)); //Initialize entries to store array of strings 
	int i;
	for(i = 0; i < 8; i++){
		rf->entries[i] = malloc(256 * sizeof(char)); //Initialize entries array for storing values read in from file
	}
}

//Sets gameState to default values
void initGame(struct gameState* gs){
	gs->steps = 0; //Set step count to 0 so that it can be used to count the number of steps taken by the player toward victory
	gs->visited = malloc(32 * sizeof(char*)); //Initialize visited to store array of strings
	int i;
	for(i = 0; i < 32; i++){
		gs->visited[i] = malloc(256 * sizeof(char)); //Initialize visited array for storing names of rooms visited during gameplay
	}
}

//Takes file descriptor and uses it to read contents of file into a roomFile struct
void getData(struct roomFile* rf, int fd){
	char buff[256];
	char word[32];
	char* mode = malloc(sizeof(char));
	*mode = 'r'; //Sets fdopen operation mode to read only
	FILE* stream = fdopen(fd, mode); //Open room file as stream for reading
	while(fgets(buff, 256, stream) != NULL){ //While there are line remaining, get one line at a time from room file
		memset(word, '\0', 32); //Reset word each iteration to prepare it to store key data from each line
		sscanf(buff, "%*s %*s %s", word); //Select the third space seperated string from each line and store it in word
		strcpy(rf->entries[rf->numEntries], word); //Place key data from each line into most recent room file entry
		rf->numEntries++; //Increment to keep track of how many entries have been read in
		memset(buff, '\0', 256);
	}
	free(mode);
}

//Reset roomFile struct to facilitate reading data from next file
void reset(struct roomFile* rf){
	int i;
	for(i = 0; i < rf->numEntries; i++){ //Use numEntries to determine how many entries need to be reset to '\0'
		memset(rf->entries[i], '\0', 256);
	} 
	rf->numEntries = 0; //Reset count, now that it is no longer needed
}

//Iterates through each entry in a given directory opening and reading files into a roomFile struct. After reading a file,
//if opt == 0, compares room name to str, if opt == 1, compares room type to str. In either case, if the strings match, 
//the roomFile struct is returned with the matching room data
struct roomFile* findRoom(struct roomFile* rf, DIR* dir, char* path, char* str, int opt){
	rewinddir(dir); //Start search files from the top of the directory each time
	struct dirent* curFile; 
	char* filePath = malloc(256 * sizeof(char));
	while((curFile = readdir(dir)) != NULL){ //Loop through each item in directory
		if(strcmp(curFile->d_name, ".") != 0 && strcmp(curFile->d_name, "..") != 0){ //Do not examine "." or ".."	
			memset(filePath, '\0', 256 * sizeof(char));	
			sprintf(filePath, "%s/%s", path, curFile->d_name); //Assemble path to file for opening 
			int fd = open(filePath, O_RDONLY); //Open room file for reading
			if(fd == -1){
				printf("Failed to open file in game directory\n");
				perror("While opening file");
				exit(1);
			}
			getData(rf, fd); //Read data from room file into roomFile struct
			if(opt == 0){
				if(strcmp(str, rf->entries[0]) == 0){ //If option 0 was specified, check if the current file's name matches a given string
					return rf;
				}
			}
			else if(opt == 1){
				if(strcmp(str, rf->entries[rf->numEntries - 1]) == 0){ //If option 1 was specified, check if the current file's type matches a given string
					return rf;
				}
			}
			reset(rf); //Reset roomFile struct so that it can be used for reading in data from the next file if not match has been found yet
		}
	} 
	return NULL;
}

//Takes a roomfile struct and presents the data it contains in a formatted prompt to the player
void prompt(struct roomFile* rf){
	printf("CURRENT LOCATION: %s\n", rf->entries[0]); //Presents the name of the current room to the player
	printf("POSSIBLE CONNECTIONS: ");
	int i;
	for(i = 1; i < rf->numEntries - 3; i++){ //Loops through all but the last of the current room's connections and lists them seperated by a comma and space 
		printf("%s, ", rf->entries[i]);
	}
	printf("%s.\n", rf->entries[rf->numEntries -2]); //Prints the last of the current room's connections, followed by a period and newline
	printf("WHERE TO? >"); //Presents prompt to the player
}

//Compares input from user to each of the current room's connections to determine if the user entered a valid option
int validateRoom(struct roomFile* rf, char* rmName){
	int i;
	for(i = 1; i < rf->numEntries - 1; i++){
		if(strcmp(rf->entries[i], rmName) == 0){ 
			return 1; //Returns 1 if the player entered a valid room name
		}
	}
	return 0; //Returns 0 if the player entered an invalid room name
}

//Prints victory message and gameplay details
void victory(struct gameState* g, struct roomFile* rf){
	printf("\nYOU HAVE FOUND THE END ROOM. CONGRADULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", g->steps);
	int i;
	for(i = 0; i < g->steps; i++){
		printf("%s\n", g->visited[i]); //Lists the rooms that were visited during gameplay, in the order that they were visited
	}
}

//Locks the myMute mutex, gets the current date/time, formats it, and writes it to time.txt, as soon as myMute is unlocked 
void* getTime(void *arg){
	pthread_mutex_lock(&myMute); //Locks myMute, as soon as it is unlocked by the main thread
	char* root = (char*) arg;
	char* path = malloc(256 * sizeof(char));
	sprintf(path, "%s/time.txt", root); //Assemble path to time.txt file
	char curTime[64] = {'\0'};
	
	time_t t;
	t = time(NULL); //Get current time
	struct tm* bdt = localtime(&t); //Convert raw time into broken-down time
	strftime(curTime, 64, "%I:%M%P, %A, %B %d %Y %n", bdt); //Takes broken-down time and formats it for output
	int fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644); //Open time.txt for writing. If it doesn't exist, create it, if it does, overwrite it
	write(fd, curTime, strlen(curTime) * sizeof(char)); //Write formatted time to time.txt
	close(fd); //Close time.txt
	pthread_mutex_unlock(&myMute); //Unlock mutex to yield control back to main
	return NULL;
}

int main()
{
	pthread_t thread; 
	pthread_mutex_lock(&myMute); //Lock mutex to prevent getTime from running
	
	char* dir = selectDir();
	struct gameState* game = malloc(sizeof(struct gameState));
	initGame(game); //Initialize game state 
	struct roomFile* roomFile = malloc(sizeof(struct roomFile));
	initFile(roomFile); //Initialize roomFile struct
	char dirPath[256] = {'\0'};
	sprintf(dirPath, "./%s", dir); //Compose path to game directory
	pthread_create(&thread, NULL, getTime, dirPath); //Create thread for getting time
	pthread_tryjoin_np(thread, NULL); //Attempt to join thread without blocking
	DIR* gameDir = opendir(dirPath); 	
	roomFile = findRoom(roomFile, gameDir, dirPath, "START_ROOM", 1); //Find start room file and load data into roomFile struct
	if(roomFile == NULL){
		printf("Could not locate room\n");
		exit(1);
	}
	size_t buffSize = 0;
	char* userInput = NULL;
	while(strcmp(roomFile->entries[roomFile->numEntries - 1], "END_ROOM") != 0){ //Continue game until room file with "END_ROOM" type is found
		prompt(roomFile); //Present player with prompt
		getline(&userInput, &buffSize, stdin); //Get user input
		int len = strlen(userInput);
		userInput[len - 1] = '\0'; //Replace '\n' in user input with '\0'
		if(strcmp(userInput, "time") == 0){ //Determines if the player has entered "time" at the prompt
			pthread_mutex_unlock(&myMute); //Unlocks mutex when player enters "time" at prompt to allow time getting thread to operate
			usleep(1000); //Pauses long enough to allow time getting thread to lock mutex, but not long enough for player to notice a delay
			pthread_mutex_lock(&myMute); //Locks the mutex again once the time getting thread unlocks it
			char timePath[256] = {0};
			sprintf(timePath, "%s/time.txt", dirPath); //Compose path to time.txt
			char* mode = malloc(sizeof(char) * 10);
			*mode = 'r'; //Set mode for opening file stream to read only
			int fd = open(timePath, O_RDONLY); //open time.txt
			FILE* stream = fdopen(fd, mode); //Load time.txt into a file stream
			char word[64] = {'\0'};
			fgets(word, 64, stream); //Get the first line of time.txt and store it in word
			printf("\n %s\n", word); //Print the formatted time data to console
			close(fd); //close time.txt
		}
		else if(validateRoom(roomFile, userInput) == 1){ //Determine if the player entered the name of a valid connecting room
			strcpy(game->visited[game->steps], userInput); //Record valid connecting room choice as a visited room
			game->steps++; //Increment the number of steps the user has taken toward victory
			reset(roomFile); //Prepare roomfile struct to store data for the connecting room that the player chose
			roomFile = findRoom(roomFile, gameDir, dirPath, userInput, 0); //Gets data for new current room
		}
		else{
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n"); //Presents player with error message if invalid choice was entered
		}	
	}
	victory(game, roomFile); //Declares victory if "END_ROOM" is reached
	closedir(gameDir);
	return 0;
}
