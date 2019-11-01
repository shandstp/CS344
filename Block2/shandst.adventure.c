#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

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
		gs->visited[i] = malloc(32 * sizeof(char));
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
//			printf("The file path is %s\n", filePath);
			if(fd == -1){
				printf("Failed to open file in game directory\n");
				perror("While opening file");
				exit(1);
			}
			getData(rf, fd);
//			int i;
//			for(i = 0; i < roomFile->numEntries; i++){
//				printf("%s ", roomFile->entries[i]);
//			}
//			printf("\n");
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

int main()
{
	char* dir = selectDir();
	printf("The most recent dir is: %s\n", dir);
	struct roomFile* roomFile = malloc(sizeof(struct roomFile));
	initFile(roomFile);
	char dirPath[256] = {'\0'};
	sprintf(dirPath, "./%s", dir);
	DIR* gameDir = opendir(dirPath); 	
	roomFile = findRoom(roomFile, gameDir, dirPath, "Hall", 0);
	if(roomFile == NULL){
		printf("Could not locate room\n");
		exit(1);
	}
	printf("Located %s\n", roomFile->entries[0]);
	closedir(gameDir);
	return 0;
}
/*
	int i;
	for(i = 0; i < dungeon->numRooms; i++){
		char path[32] = {'\0'};
		sprintf(path, "%s/%s", curDir, dungeon->dungeonMap[i]->rmName);
		int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
		if(fd == -1){
			printf("Failed to open file\n");
			perror("While opening file");
			exit(1);
		}
	
		char* output = calloc(32, sizeof(char));
		memset(output, '\0', 32);
		sprintf(output, "ROOM NAME: %s\n", dungeon->dungeonMap[i]->rmName);
		write(fd, output, strlen(output) * sizeof(char));
		int j;
		for(j = 0; j < dungeon->dungeonMap[i]->numConnect; j++){
			memset(output, '\0', 32);
			sprintf(output, "CONNECTION %d: %s\n", j + 1, dungeon->dungeonMap[i]->outbound[j]->rmName);
			write(fd, output, strlen(output) * sizeof(char));
		}
		memset(output, '\0', 32);
		sprintf(output, "ROOM TYPE: %s\n", dungeon->dungeonMap[i]->type);
		write(fd, output, strlen(output) * sizeof(char));
*/
