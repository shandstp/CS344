#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

struct Files{
	int numEntries;
	char** enties;
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

int main()
{
	char* dir = selectDir();
	printf("The most recent dir is: %s\n", dir);
	return 0;
}
