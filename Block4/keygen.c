#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int main(int argc, char* argv[]){
	srand(time(0));
	if(argc != 2){
		errno = EINVAL;
		perror("Invalid number of arguments");	
		exit(1);
	}
	char valChar[27] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int keylen = atoi(argv[1]) + 1;
	char* key = malloc(sizeof(char) * keylen);
	for(int i = 0; i < keylen - 1; i++){
		key[i] = valChar[rand() % 27];
	}
	key[keylen - 1] = '\n';
	printf("%s", key);
	
	return 0;
}
