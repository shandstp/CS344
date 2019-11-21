#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

struct cmd{
	int bgflag;
	int status;
	char* infile;
	char* outfile;
	int numArgs;
	char* args[512];
};

void initCmd(struct cmd* cur){
	cur->bgflag = 0;
	cur->status = 0;
	cur->infile = malloc(sizeof(char) * 32);
	cur->outfile = malloc(sizeof(char) * 32);
	strcpy(cur->infile, "");
	strcpy(cur->outfile, "");
//	int defaultIN = open("/dev/stdin", O_WRONLY);
//	int defaultOUT = open("/dev/stdout", O_RDONLY);
//	dup2(defaultIN, 0);
//	dup2(defaultOUT, 1);
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
			else if(input[i + 1] == '\0' && strcmp(word, "&") == 0){
				cur->bgflag = 1;
			}
			else{
				strcpy(cur->args[cur->numArgs], word);
				cur->numArgs++;
			}
			ws = i + 1;
		}
		i++;
	}
	free(cur->args[cur->numArgs]);	
	cur->args[cur->numArgs] = NULL;
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
	if(cur->numArgs == 1){
		char* home = getenv("HOME");
		cur->status = chdir(home);
	}
	else{
		cur->status = chdir(cur->args[1]);
	}	
}

void getStatus(struct cmd* cur){
	char sstat[10];
	memset(sstat, '\0', 10);
	sprintf(sstat, "%d\n", cur->status);
	write(1, sstat, 10);
	fflush(stdout);
}	

pid_t kodzukuri(struct cmd* cur){
	pid_t spawnPid = -5;
	int childExitStatus = -5;

	spawnPid = fork();
	switch(spawnPid){
		case -1:
			perror("Something's forked up\n"); exit(1); break;
		case 0: {
	int infd = -5;
	if((strcmp(cur->infile, "") != 0)){
		if((infd = open(cur->infile, O_RDONLY)) != -1){
			//close(0);
			dup2(infd, 0);
		}	
		else{
			perror("Invalid input file!");
			exit(1);
		}
	}
	int outfd = -5;
	if((strcmp(cur->outfile, "") != 0)){
		if((outfd = open(cur->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644)) != -1){
			//close(1);
			dup2(outfd, 1);
		}
	}
	if(cur->bgflag == 1){
		int voidRead = open("/dev/null", O_RDONLY);
		int voidWrite = open("/dev/null", O_WRONLY);
		//close(0);
		//close(1);
		dup2(voidRead, 0);
		dup2(voidWrite, 1);
	}	
			char** argv = cur->args;
			if(execvp(*argv, argv) < 0){
				perror("Exec had a problem\n");
				exit(1);
			}
			break;	
		}
		default: {
			pid_t actualPid = -5;
			if(cur->bgflag == 1){
				actualPid = waitpid(spawnPid, &childExitStatus, WNOHANG);
			}
			else{
				actualPid = waitpid(spawnPid, &childExitStatus, 0);
			}
			return actualPid;
		}
	}
	return -1; 
}

void freeCmd(struct cmd* cur){
	free(cur->infile);
	free(cur->outfile);
	for(int i = 0; i < 512; i++){
		free(cur->args[i]);
	}
	free(cur);
}

int main(){
	//Loop until user types exit
	while(1){

//		int defaultIn = open("/dev/stdin", O_RDONLY);
//		int defaultOut = open("/dev/stdout", O_WRONLY);
		dup2(2, 0);
		dup2(2, 1);
		int childExitStatus = -5;
		pid_t actualPid = waitpid(-1, &childExitStatus, WNOHANG);
		if(WIFEXITED(childExitStatus)){
			char childExit[16];
			memset(childExit, '\0', 16);
			sprintf(childExit, "%d\n", actualPid);
			write(1, childExit, 16);
			fflush(stdout);
		}

		char* prompt = ": "; //User prompt
		write(1, prompt, 3); //Print prompt
		fflush(stdout);
		char* input = malloc(sizeof(char) * 2048); //Allocate space for input
		memset(input, '\0', 2048);
//		size_t bsize = 0;
//		char* buff = NULL;
//		getline(&buff, &bsize, stdin);
		char ch[2] = {'\0'};
		while(ch[0] != '\n'){
			read(0, ch, sizeof(char)); //Get input from user
			strcat(input, ch);
		}
		struct cmd* cur = malloc(sizeof(struct cmd));
		initCmd(cur);	
		buildCmd(cur, input);	
//		printCmd(cur);
		if(cur->args[0][0] ==  '#'){
			char* scream = "You see nothing!";
			int thevoid = open("/dev/null", O_WRONLY);
			write(thevoid, scream, 16);
			fflush(stdout);
		}
		else if(strcmp(cur->args[0], "") == 0){
			int thevoid = open("/dev/null", O_WRONLY);
			write(thevoid, "Do nothing", 10);
			fflush(stdout);
		} 
		else if(strcmp(cur->args[0], "exit") == 0){
			freeCmd(cur);
			free(input);
			exit(0);
		}
		else if(strcmp(cur->args[0], "cd") == 0){
			chgdir(cur);
		}
		else if(strcmp(cur->args[0], "status") == 0){
			getStatus(cur);	
		}
		else{
			kodzukuri(cur);
		}
		freeCmd(cur);
		free(input);
	}
	return 0;
}
