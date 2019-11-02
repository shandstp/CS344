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

struct gameState{
	int steps;
	char** visited;
};

struct roomFile{
	int numEntries;
	char** entries;
};

char* selectDir(){

	int mostRecent = -1;
	char dirPrefix[32] = "shandst.rooms.";
	int nameLen = 256;
	char* newDir = malloc(nameLen * sizeof(char));
	memset(newDir, '\0', nameLen);

	DIR* topDir;
	struct dirent *curDir;
	struct stat dirInfo;

	topDir = opendir(".");
	if(topDir > 0){
		while((curDir = readdir(topDir)) != NULL){
			if(strstr(curDir->d_name, dirPrefix) != NULL){
				stat(curDir->d_name, &dirInfo);
				
				if((int)dirInfo.st_mtime > mostRecent){
					mostRecent = (int)dirInfo.st_mtime;
					memset(newDir, '\0', nameLen);
					strcpy(newDir, curDir->d_name);
				}
			}
		}
	}


	closedir(topDir);
	return newDir;
}

void initFile(struct roomFile* rf){
	rf->numEntries = 0;
	rf->entries = malloc(8 * sizeof(char*));
	int i;
	for(i = 0; i < 8; i++){
		rf->entries[i] = malloc(256 * sizeof(char));
	}
}

void initGame(struct gameState* gs){
	gs->steps = 0;
	gs->visited = malloc(32 * sizeof(char*));
	int i;
	for(i = 0; i < 32; i++){
		gs->visited[i] = malloc(256 * sizeof(char));
	}
}

void getData(struct roomFile* rf, int fd){
	char buff[256];
	char word[32];
	char* mode = malloc(sizeof(char));
	*mode = 'r';
	FILE* stream = fdopen(fd, mode);
	while(fgets(buff, 256, stream) != NULL){
		memset(word, '\0', 32);
		sscanf(buff, "%*s %*s %s", word);
		strcpy(rf->entries[rf->numEntries], word);
		rf->numEntries++;
		memset(buff, '\0', 256);
	}
	free(mode);
}

void reset(struct roomFile* rf){
	int i;
	for(i = 0; i < rf->numEntries; i++){
		memset(rf->entries[i], '\0', 256);
	} 
	rf->numEntries = 0;
}

struct roomFile* findRoom(struct roomFile* rf, DIR* dir, char* path, char* str, int opt){
	rewinddir(dir);
	struct dirent* curFile;
	char* filePath = malloc(256 * sizeof(char));
	while((curFile = readdir(dir)) != NULL){
		if(strcmp(curFile->d_name, ".") != 0 && strcmp(curFile->d_name, "..") != 0){	
			memset(filePath, '\0', 256 * sizeof(char));	
			sprintf(filePath, "%s/%s", path, curFile->d_name);
			int fd = open(filePath, O_RDONLY);
			if(fd == -1){
				printf("Failed to open file in game directory\n");
				perror("While opening file");
				exit(1);
			}
			getData(rf, fd);
			if(opt == 0){
				if(strcmp(str, rf->entries[0]) == 0){
					return rf;
				}
			}
			else if(opt == 1){
				if(strcmp(str, rf->entries[rf->numEntries - 1]) == 0){
					return rf;
				}
			}
			reset(rf);
		}
	} 
	return NULL;
}

void prompt(struct roomFile* rf){
	printf("CURRENT LOCATION: %s\n", rf->entries[0]);
	printf("POSSIBLE CONNECTIONS: ");
	int i;
	for(i = 1; i < rf->numEntries - 3; i++){
		printf("%s, ", rf->entries[i]);
	}
	printf("%s.\n", rf->entries[rf->numEntries -2]);
	printf("WHERE TO? >");
}

int validateRoom(struct roomFile* rf, char* rmName){
	int i;
	for(i = 1; i < rf->numEntries - 1; i++){
		if(strcmp(rf->entries[i], rmName) == 0){
			return 1;
		}
	}
	return 0;
}

void victory(struct gameState* g, struct roomFile* rf){
	printf("YOU HAVE FOUND THE END ROOM. CONGRADULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", g->steps);
	int i;
	for(i = 0; i < g->steps; i++){
		printf("%s\n", g->visited[i]);
	}
}

int main()
{
	char* dir = selectDir();
	struct gameState* game = malloc(sizeof(struct gameState));
	initGame(game);
	struct roomFile* roomFile = malloc(sizeof(struct roomFile));
	initFile(roomFile);
	char dirPath[256] = {'\0'};
	sprintf(dirPath, "./%s", dir);
	DIR* gameDir = opendir(dirPath); 	
	roomFile = findRoom(roomFile, gameDir, dirPath, "START_ROOM", 1);
	if(roomFile == NULL){
		printf("Could not locate room\n");
		exit(1);
	}
	size_t buffSize = 0;
	char* userInput = NULL;
	while(strcmp(roomFile->entries[roomFile->numEntries - 1], "END_ROOM") != 0){
		prompt(roomFile);
		getline(&userInput, &buffSize, stdin);
		int len = strlen(userInput);
		userInput[len - 1] = '\0';
		if(validateRoom(roomFile, userInput) == 1){
			strcpy(game->visited[game->steps], userInput);
			game->steps++;
			initFile(roomFile);
			roomFile = findRoom(roomFile, gameDir, dirPath, userInput, 0);
		}
		else{
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}	
	}
	victory(game, roomFile);
	closedir(gameDir);
	return 0;
}
