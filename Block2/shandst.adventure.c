#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

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

int main()
{
	char* dir = selectDir();
	printf("The most recent dir is: %s\n", dir);
	struct roomFile* curFile = NULL;
	initFile(curFile);
	char dirPath[256] = {'\0'};
	sprintf(dirPath, "./%s", dir);
	DIR* gameDir = opendir(dirPath); 	
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
