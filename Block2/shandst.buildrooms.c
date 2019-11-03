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



enum truth {false, true}; //Allow use of true and false to improve readability 

//Facilitates the storing of room data as it is being randomly generated
struct Room {
	int id; //Used to easily compare rooms without neading to compare strings
	char* type; //Used to identify a room as START_ROOM, END_ROOM, or MID_ROOM
	int numConnect; //Tracks number of rooms that the each is connected to
	char* rmName; //Stores name of each room
	struct Room* outbound[6]; //An array of Room struct pointers for storing the address of each room connected to
};

//Organizes Rooms into a larger map
struct Dungeon {
	int capacity; //Tracks how many rooms can contained within each dungeon
	int numRooms; //Tracks how many rooms have been added to each dungeon
	struct Room* dungeonMap[7]; //An array of Room struct pointers for organizing Rooms into a single map
};

//Takes a Room struct and initializes it to the default values of 0 connections and no name or type values
void initRoom(struct Room* room){
	room->numConnect = 0; //Sets a room struct's connection count to 0 so that room->numConnect can be used to track how many connections each room has
	//Prepare room->type and room->rmName for storing strings
	room->type = calloc(16, sizeof(char));
	room->rmName = calloc(16, sizeof(char));
	int i;
	//Prepare to store a reference to each room that this one is connected to
	for(i = 0; i < 6; i ++){
		room->outbound[i] = malloc(sizeof(struct Room));
	}
}

//Takes a Dungeon struct and initializes it to the default values of 7 capacity, 0 room count, and no name values
void initDungeon(struct Dungeon* d){
	d->capacity = 7; //Provides the limit for how many rooms can be added to a dungeon
	d->numRooms = 0; //Prepare to count how many rooms have been added to this dungeon
	int i;
	for(i = 0; i < d->capacity; i++){ //Prepares each element of the dungeon map to store a room struct
        d->dungeonMap[i] = malloc(sizeof(struct Room));
		initRoom(d->dungeonMap[i]);
	}
}

//Iterates through a dungeon and prints all of its values. Only used for testing
void printDungeon(struct Dungeon* d){
	printf("Details of the dungeon are as follows:\n");
	printf("Room capacity: %d\n", d->capacity);
	printf("Number of rooms assigned: %d\n", d->numRooms);
	int i;
	for(i = 0; i < d->capacity; i++){ //Loops through the dungeon map and prints the name of each room stored therin
		printf("Room %d is at: %p\n", i, d->dungeonMap[i]);
		if(d->dungeonMap[i] != NULL){
			printf("Name: %s\n", d->dungeonMap[i]->rmName);
		}
	}
}

//Iterates through a dungeon checking the id of each room contained therin. Returns a 1 if a room is found with an id that matches the given id, otherwise 0.
int containsRoom(struct Dungeon* d, int id){
    int i;
    for(i = 0; i < d->capacity; i++){
        if(d->dungeonMap[i]->id == id){
            return 1; //Return 1 if the dungeon already contains a rooms with the given id
        }
    }
    return 0; //Return 0 if a room with an id matching the one given was not found in the dungeon map 
}

//Takes a room id and returns the corresponding name
void getName(struct Dungeon* d, int id){
    switch (id) //Finds the room name that corresponds to the given id and then assigns both the most recently added room in the dungeon map
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
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Jail");
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
            strcpy(d->dungeonMap[d->numRooms]->rmName, "Armory");
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

//Used as the map is being generated to determine what a rooms type will be based on when it is added to the dungeon
void getType(struct Dungeon* d){
	switch(d->numRooms) //Assigns type of START_ROOM to first room in dugeon map, END_ROOM to the last room, and MID_ROOM to all others
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

//Randomly selects 7 rooms to be used in the new dungeon
void chooseRooms(struct Dungeon* d){
    while(d->numRooms < d->capacity){ //While the number of rooms in the dungeon map is less than capacity of that dungeon, keep adding rooms
        int id = (rand() % 10) + 1;
        if(containsRoom(d, id) != 1){ //Only add room if the dungeon map does not already contain a room with the current id
            getName(d, id); //Name newest room in map
	    getType(d); //Assign type to newest room in map
            d->numRooms++; //Increase count to indicate number of rooms currently in map
        }
    }
}

//Determines if each room in dungeon map has at least three connections 
int fullyConnected(struct Dungeon* d){
	int i;
	for(i = 0; i < d->capacity; i++){
		if(d->dungeonMap[i]->numConnect < 3){
			return 0; //returns 0 as soons as a room is found that doesn't have at least 3 connections
		}
	}
	return 1; //returns 1 if all rooms in dungeon map have at least 3 connections
}

//Determines if a connection can be added to the given room
int hasRoom(struct Room* r){
	if(r->numConnect < 6){
		return 1; //Returns 1 if the given room has rooms for another connection
	}
	return 0; //Returns 0 if the given room cannot be connected to any more rooms
}

//Determines if the room corresponding to x's id has already been added to y's room connections
int alreadyAdded(struct Dungeon* d, int x, int y){
	int i;
	for(i = 0; i < d->dungeonMap[x]->numConnect; i++){
		if(d->dungeonMap[y]->id == d->dungeonMap[x]->outbound[i]->id){
			return 1; //returns 1 if the rooms at index x of dungeon map is already connected to the room at index y
		}
	}	
	return 0; //returns 0 if rooms at index x and y of dungeon map have not already been connected
}

//Takes two rooms and adds each to the others connection list
void connectRooms(struct Room* r1, struct Room* r2){
	r1->outbound[r1->numConnect] = r2; //Adds room 2 to the connection list of room 1 and increments room 1's connection number
	r1->numConnect++;
	r2->outbound[r2->numConnect] = r1; //Adds room 1 to the connection list of room 2 and increments room 2's connection number
	r2->numConnect++;	
}

//Determines which rooms can be connected togeter until all of the rooms have been sufficiently connected
void connectDungeon(struct Dungeon* d){
	while(fullyConnected(d) != true){ //While there are rooms in dungeon map with fewer than 3 connections continue to connect rooms at random
		int x = rand() % d->capacity;	
		int y = rand() % d->capacity;
		while(hasRoom(d->dungeonMap[x]) == false){ //Choose rooms at random until one is found that has room for more connections
			x = rand() % d->capacity; 	
		}
		do{ //Choose rooms at random until one is found that can be connected to the one at index x
			y = rand() % d->capacity;
		} while(x == y || hasRoom(d->dungeonMap[y]) == false || alreadyAdded(d, x, y) == true);
		
		connectRooms(d->dungeonMap[x], d->dungeonMap[y]); //Connect the selected rooms to each other 
	}
}

//Prints details for all of the rooms in the given dungeon. Only used for testing
void printRooms(struct Dungeon* d){
   int i;
   for(i = 0; i < 7; i++){ //Print the name of each room in dungeon map and the names of each room it is connected to
   	printf("%s is a %s and is connected to:\n", d->dungeonMap[i]->type, d->dungeonMap[i]->rmName); 
       	int j;
       	for(j = 0; j < 6; j++){
            printf("%s ", d->dungeonMap[i]->outbound[j]->rmName); //Print the name of each room that this one is connected to
       
        }
        printf("\n");
    }
}

//Creats a new directory and returns its name
char* newDir(){
    int procid = getpid(); 
    char dirPrfx [32] = "shandst.rooms.";
    char* dirName = calloc(32, sizeof(char));
    sprintf(dirName, "%s%d", dirPrfx, procid); //Smash the directory prefix and current process id together into a unique directory name
    mkdir(dirName, 0776); //Make uniquely named directory

    return dirName;
}

int main(){

    	srand(time(0)); //Seed random number generation for selecting random rooms and their connections

	struct Dungeon* dungeon = malloc(sizeof(struct Dungeon)); //Reserve memory on the heap for a dungeon struct
	initDungeon(dungeon); //Initialize the dungeon to default values
    	chooseRooms(dungeon); //Randomly select 7 rooms and add them to the dungeon
	connectDungeon(dungeon); //Randomly connect each room in the dungeon to at least 3 others but no more than 6

	char* curDir = newDir();
	int i;
	for(i = 0; i < dungeon->numRooms; i++){ //Increment through each room in the dungeon and write its details to the corresponding file in the newly created shandst.rooms.<PID> folder
		char path[32] = {'\0'}; //Prepare to store the path to the soon to be created room file
		sprintf(path, "%s/%s", curDir, dungeon->dungeonMap[i]->rmName); //Build and store the path to be used for creating the current rooms new file
		int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644); //Create new file room
		if(fd == -1){ //Report failure to create room file and exit
			printf("Failed to open file\n");
			perror("While opening file");
			exit(1);
		}
	
		char* output = calloc(32, sizeof(char));
		memset(output, '\0', 32); //Prepare string for each use
		sprintf(output, "ROOM NAME: %s\n", dungeon->dungeonMap[i]->rmName); //Prepare the string 'ROOM NAME: <current room>\n' to be written to the start of the current file
		write(fd, output, strlen(output) * sizeof(char)); //Write room name to file
		int j;
		for(j = 0; j < dungeon->dungeonMap[i]->numConnect; j++){ //Prepare and append strings of format 'CONNECTION <number>: <room name>\n' to current file
			memset(output, '\0', 32); //Prepare string for each use
			sprintf(output, "CONNECTION %d: %s\n", j + 1, dungeon->dungeonMap[i]->outbound[j]->rmName);
			write(fd, output, strlen(output) * sizeof(char)); //Writes each of a rooms connections to its own line in the file
		}
		memset(output, '\0', 32); //Prepare string for re-use
		sprintf(output, "ROOM TYPE: %s\n", dungeon->dungeonMap[i]->type); //Prepare the string 'ROOM TYPE: <type>\n' to be appended to the end of the file
		write(fd, output, strlen(output) * sizeof(char)); //Appends current rooms type to the last line of the current room file
	}
 
	return 0;
}
