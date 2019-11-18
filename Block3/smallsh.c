#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct cmd{
	char* infile;
	char* outfile;
	int numArgs;
	char* args[512];
};

void initCmd(struct cmd* cur){
	cur->infile = malloc(sizeof(char) * 32);
	cur->outfile = malloc(sizeof(char) * 32);
	cur->numArgs = 0;
	for(int i = 0; i < 512; i++){
		cur->args[i] = malloc(sizeof(char) * 32);
	}
}	

void getWord(char* word, int ws, int we, char* input){
	int j = 0;
	for(int i = ws; i < we; i++){
		word[j] = input[i];
		j++;
	}
}

void expand(char* word){
	int pid = getpid();
	char spid[32];
	sprintf(spid, "%d", pid);
	memset(word, '\0', 32);
	strcpy(word, spid);
}

void buildCmd(struct cmd* cur, char* input){
	int i = 0;
	int ws = 0; //word start
	int inflag = 0;
	int outflag = 0;
	char word[32];
	while(input[i] != '\0'){
		if(input[i] == ' ' || input[i + 1] == '\0'){
			memset(word, '\0', 32);
			getWord(word, ws, i, input);	
			if(strcmp(word, "<") == 0){
				inflag = 1;
			}
			else if(strcmp(word, ">") == 0){
				outflag = 1;
			}
			else if(inflag == 1){
				strcpy(cur->infile, word);
				inflag = 0;
			}
			else if(outflag == 1){
				strcpy(cur->outfile, word);
				outflag = 0;
			}
			else if(strcmp(word, "$$") == 0){
				expand(word);
				strcpy(cur->args[cur->numArgs], word);
				cur->numArgs++;
			}
			else{
				strcpy(cur->args[cur->numArgs], word);
				cur->numArgs++;
			}
			ws = i + 1;
		}
		i++;
	}
}

void printCmd(struct cmd* cur){
	printf("The input is: %s\n", cur->infile);
	printf("The output is: %s\n", cur->outfile);
	printf("The arguments are as follows:\n");
	for(int i = 0; i < cur->numArgs; i++){
		printf("%s ", cur->args[i]);
	}
	printf("\n");
	fflush(stdout);
}
		
void chgdir(struct cmd* cur){

}
		
int main(){
	//Loop until user types exit
	while(1){

		char* prompt = ": "; //User prompt
		write(1, prompt, 3); //Print prompt
		fflush(stdout);
		char* input = malloc(sizeof(char) * 2048); //Allocate space for input
		read(0, input, sizeof(char) * 2048); //Get input from user

		struct cmd* cur = malloc(sizeof(struct cmd));
		initCmd(cur);	
		buildCmd(cur, input);	
		printCmd(cur);
		if(strcmp(cur->args[0], "exit") == 0){
			exit(0);
		}
		else if(strcmp(cur->args[0], "cd") == 0){
			chgdir(cur);
		}
	}
	return 0;
}
