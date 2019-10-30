#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

struct room{
	int id;
	int numConnect;
	char* roomName;
	struct room* connection[6];
};


int isGraphFull(struct room* dungeon){
	int i;
	for(i = 0; i < 7; i++){
		if(dungeon[i].numConnect < 3){
			return 0;
		}
	}
	return 1;
}

int getRand(int bound){
	srand(time(0));
	return rand() % bound;
}

int canAddConnectionFrom(struct room* dungeon, int a){
	if(dungeon[a].numConnect < 6){
		return 1;
	}
	else{
		return 0;
	}	
}

int connectionAlreadyExists(struct room* dungeon, int a, int b){
	int i;
	for(i = 0; i < dungeon[a].numConnect; i++){
		if(dungeon[a].connection[i] == &dungeon[b]){
			return 1;
		}
	}
	return 0;
}

void connectRoom(struct room* dungeon, int a, int b){
	dungeon[a].connection[dungeon[a].numConnect] = &dungeon[b];
	dungeon[b].connection[dungeon[b].numConnect] = &dungeon[a];
	dungeon[a].numConnect++;
	dungeon[b].numConnect++;
}

int isSameRoom(struct room* dungeon, int a, int b){
	if(dungeon[a].id == dungeon[b].id){
		return 1;
	}
	else{
		return 0;
	}
}

void addRandomConnection(struct room* dungeon){
	//struct room a;
	//struct room b;
	int a;
	int b;

	while(1){
		a = getRand(7);
		if(canAddConnectionFrom(dungeon, a) == 1) break;
	}

	do{
		b = getRand(7);

	}
	while(canAddConnectionFrom(dungeon, b) == 0 || isSameRoom(dungeon, a, b) == 1 || connectionAlreadyExists(dungeon, a, b) == 1);

	connectRoom(dungeon, a, b);
	connectRoom(dungeon, b, a);
}

void buildGraphs(struct room* dungeon){
	while(isGraphFull(dungeon) == 0){
		addRandomConnection(dungeon);
	}
}

int* chooseRooms(){
	int static roomNums [7];
	int numChosen = 0;
	while(numChosen < 7){
		int numChoice = getRand(10);
		int used = 0;
		int i;
		for(i = 0; i < numChosen; i++){
			if(roomNums[i] == numChoice){
				used = 1;
			}
		}
		if(used == 0){
			roomNums[numChosen] = numChoice;
			numChosen++;
		}
	}
	return roomNums;
}

void makeDungeon(struct room* dungeon){
	int* roomNums = chooseRooms();
	int i;
	for(i = 0; i < 7; i++){
		dungeon[i].numConnect = 0;
		switch(roomNums[i]){
			case 0:
				dungeon[i].id = i;
				dungeon[i].roomName = "Arboretum";	
				break;
			case 1:
				dungeon[i].id = i;
				dungeon[i].roomName = "Library";
				break;
			case 2:	
				dungeon[i].id = i;
				dungeon[i].roomName = "Barracks";
				break;
			case 3:	
				dungeon[i].id = i;
				dungeon[i].roomName = "The Pit";
				break;
			case 4:	
				dungeon[i].id = i;
				dungeon[i].roomName = "The Lab";
				break;
			case 5:	
				dungeon[i].id = i;
				dungeon[i].roomName = "Courtyard";
				break;
			case 6:	
				dungeon[i].id = i;
				dungeon[i].roomName = "The Hall";
				break;
			case 7:	
				dungeon[i].id = i;
				dungeon[i].roomName = "Scriptorium";
				break;
			case 8:	
				dungeon[i].id = i;
				dungeon[i].roomName = "The Gallows";
				break;
			case 9:	
				dungeon[i].id = i;
				dungeon[i].roomName = "Scrying Chamber";
				break;
		}
	}
} 

int main(int argc, char *argv[]){

	struct room dungeon[7];
	makeDungeon(dungeon);	
	buildGraphs(dungeon);
	int i;
	printf("The rooms map out as follows:\n");
	for(i = 0; i < 7; i++){
		printf("%s is connected to ", dungeon[i].roomName);
		int j;
		for(j = 0; j < dungeon[i].numConnect; j++){
			printf("%s ", dungeon[i].connection[j]->roomName);	
		}
		printf("\n");
	}
	printf("\n");
	return 0;
}
