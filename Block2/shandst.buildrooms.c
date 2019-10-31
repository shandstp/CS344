#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>



enum truth {false, true};

struct Room {
	int id;
	char* type;
	int numConnect;
	char* rmName;
	struct Room* outbound[6];
};

struct Dungeon {
	int capacity;
	int numRooms;
	struct Room* dungeonMap[7];
};

void initRoom(struct Room* room){
	room->numConnect = 0;
	room->type = calloc(16, sizeof(char));
	room->rmName = calloc(16, sizeof(char));
	int i;
	for(i = 0; i < 6; i ++){
		room->outbound[i] = malloc(sizeof(struct Room));
	}
}

void initDungeon(struct Dungeon* d){
	d->capacity = 7;
	d->numRooms = 0;
	int i;
	for(i = 0; i < d->capacity; i++){
        d->dungeonMap[i] = malloc(sizeof(struct Room));
		initRoom(d->dungeonMap[i]);
	}
}

void printDungeon(struct Dungeon* d){
	printf("Details of the dungeon are as follows:\n");
	printf("Room capacity: %d\n", d->capacity);
	printf("Number of rooms assigned: %d\n", d->numRooms);
	int i;
	for(i = 0; i < d->capacity; i++){
		printf("Room %d is at: %p\n", i, d->dungeonMap[i]);
		if(d->dungeonMap[i] != NULL){
			printf("Name: %s\n", d->dungeonMap[i]->rmName);
		}
	}
}

int containsRoom(struct Dungeon* d, int id){
    int i;
    for(i = 0; i < d->capacity; i++){
        if(d->dungeonMap[i]->id == id){
            return 1;
        }
    }
    return 0;
}

void getName(struct Dungeon* d, int id){
    switch (id)
    {
        case 1:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Crypt");
            break;
        case 2:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Cellar");
            break;
        case 3:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Library");
            break;
        case 4:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Colosseum");
            break;
        case 5:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Wall");
            break;
        case 6:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Hall");
            break;
        case 7:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Battlement");
            break;
        case 8:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Hole");
            break;
        case 9:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Attic");
            break;
        case 10:
            d->dungeonMap[d->numRooms]->id = id;
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Garden");
            break;
    }
}

void getType(struct Dungeon* d){
	switch(d->numRooms)
	{
		case 0:
			strcpy(d->dungeonMap[d->numRooms]->type, "START_ROOM");
			break;
		case 6:
			strcpy(d->dungeonMap[d->numRooms]->type, "END_ROOM");
			break;
		default:
			strcpy(d->dungeonMap[d->numRooms]->type, "MID_ROOM");
			break;
	}
}

void chooseRooms(struct Dungeon* d){
    while(d->numRooms < d->capacity){
        int id = (rand() % 10) + 1;
        if(containsRoom(d, id) != 1){
            getName(d, id);
	    getType(d);
            d->numRooms++;
        }
    }
}

int fullyConnected(struct Dungeon* d){
	int i;
	for(i = 0; i < d->capacity; i++){
		if(d->dungeonMap[i]->numConnect < 3){
			return 0;
		}
	}
	return 1;
}

int hasRoom(struct Room* r){
	if(r->numConnect < 6){
		return 1;
	}
	return 0;
}

int alreadyAdded(struct Dungeon* d, int x, int y){
	int i;
	for(i = 0; i < d->dungeonMap[x]->numConnect; i++){
		if(d->dungeonMap[y]->id == d->dungeonMap[x]->outbound[i]->id){
			return 1;
		}
	}	
	return 0;
}

void connectRooms(struct Room* r1, struct Room* r2){
	r1->outbound[r1->numConnect] = r2;
	r1->numConnect++;
	r2->outbound[r2->numConnect] = r1;
	r2->numConnect++;	
}

void connectDungeon(struct Dungeon* d){
	while(fullyConnected(d) != true){
		int x = rand() % d->capacity;	
		int y = rand() % d->capacity;
		while(hasRoom(d->dungeonMap[x]) == false){
			x = rand() % d->capacity; 	
		}
		do{
			y = rand() % d->capacity;
		} while(x == y || hasRoom(d->dungeonMap[y]) == false || alreadyAdded(d, x, y) == true);
		
		connectRooms(d->dungeonMap[x], d->dungeonMap[y]);	
	}
}

void printRooms(struct Dungeon* d){
   int i;
   for(i = 0; i < 7; i++){
   	printf("%s is a %s and is connected to:\n", d->dungeonMap[i]->type, d->dungeonMap[i]->rmName); 
       	int j;
       	for(j = 0; j < 6; j++){
            printf("%s ", d->dungeonMap[i]->outbound[j]->rmName);
       
        }
        printf("\n");
    }
}

char* newDir(){
    int procid = getpid();
    char dirPrfx [32] = "shandst.rooms.";
    char* dirName = calloc(32, sizeof(char));
    sprintf(dirName, "%s%d", dirPrfx, procid);
    mkdir(dirName, 0776);

    return dirName;
}

int main(){

    	srand(time(0));

	struct Dungeon* dungeon = malloc(sizeof(struct Dungeon));
	initDungeon(dungeon);
    	chooseRooms(dungeon);
	connectDungeon(dungeon);

	char* curDir = newDir();
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
	}
 
	return 0;
}
