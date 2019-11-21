#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int canbg = 1;

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

void expand(char* word, char* substr){
	int pid = getpid();
	char spid[32];
	sprintf(spid, "%d", pid);
	char* tail = substr + 2;
	strcat(spid, tail);
	int offset = substr - word;
	word[offset] = '\0';
	strcat(word, spid);
}

void buildCmd(struct cmd* cur, char* input){
	int i = 0;
	int ws = 0; //word start
	int inflag = 0;
	int outflag = 0;
	char word[32];
	char* substr = NULL;
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
			else if((substr = strstr(word, "$$")) != NULL){
				expand(word, substr);
				strcpy(cur->args[cur->numArgs], word);
				cur->numArgs++;
			}
			else if(input[i + 1] == '\0' && strcmp(word, "&") == 0){
				if(canbg == 1){
					cur->bgflag = 1;
				}
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
			if(cur->bgflag == 0){
				signal(SIGINT, SIG_DFL);
			}
			else{
				signal(SIGINT, SIG_IGN);
			}
			signal(SIGTSTP, SIG_IGN);
			int infd = -5;
			if((strcmp(cur->infile, "") != 0)){
				if((infd = open(cur->infile, O_RDONLY)) != -1){
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
					dup2(outfd, 1);
				}
			}
			if(cur->bgflag == 1){
				int voidRead = open("/dev/null", O_RDONLY);
				int voidWrite = open("/dev/null", O_WRONLY);
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
			if(cur->bgflag == 1){
				waitpid(spawnPid, &childExitStatus, WNOHANG);
			}
			else{
				return spawnPid;
			}
			return -5;
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

void catchSIGTSTP(int signo){
	char message[120];
	memset(message, '\0', 120);
	if(canbg == 1){
		strcpy(message, " SIGTSTP received, background processes will no longer be allowed. Press Ctrl-z again to re-enable background processes\n");
		canbg = 0;	
	}
	else{
		strcpy(message, " SIGTSTP received, background processes have been re-enabled\n");
		canbg = 1;
	}
	write(1, message, 120);
	fflush(stdout);
}

int main(){
	struct sigaction ignore_action = {{0}};
	ignore_action.sa_handler = SIG_IGN;
	sigaction(SIGINT, &ignore_action, NULL);

	struct sigaction SIGTSTP_action = {{0}};
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;

	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	//signal(SIGINT, SIG_IGN);

	//Loop until user types exit
	while(1){

		int childExitStatus = -5;
		int fgExitStatus = 0;
		//pid_t fgPid = -5;
		pid_t fgSPid = -5;
		pid_t bgPid = waitpid(-1, &childExitStatus, WNOHANG);
		if(WIFEXITED(childExitStatus)){
			char childExit[64];
			memset(childExit, '\0', 64);
			sprintf(childExit, "background pid %d is done: exit value is %d\n", bgPid, childExitStatus);
			write(1, childExit, 64);
			fflush(stdout);
		}
		/*
		if(WIFSIGNALED(fgExitStatus)){
			char fgSignalExit[32];
			memset(fgSignalExit, '\0', 32);
			sprintf(fgSignalExit, "terminated by signal %d", fgExitStatus);
			write(1, fgSignalExit, 32);
			fflush(stdout);
		}
		*/
		char* prompt = ": "; //User prompt
		write(1, prompt, 3); //Print prompt
		fflush(stdout);
		char* input = malloc(sizeof(char) * 2048); //Allocate space for input
		memset(input, '\0', 2048);
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
			fgSPid = kodzukuri(cur);
			waitpid(fgSPid, &fgExitStatus, 0);
		}
		
		freeCmd(cur);
		free(input);
	}
	return 0;
}
