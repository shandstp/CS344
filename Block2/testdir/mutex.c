#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

pthread_mutex_t myMute = PTHREAD_MUTEX_INITIALIZER;

void* perform_work(void *arg){
	
	pthread_mutex_lock(&myMute);
	char* path = "./time.txt";
	char curTime[64] = {'\0'};
	time_t t;
	t = time(NULL);
	struct tm* bdt = localtime(&t);	
	strftime(curTime, 64, "%I:%M%P, %A, %B %d %Y %n", bdt);
	int fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	write(fd, curTime, strlen(curTime) * sizeof(char));
	close(fd);
	pthread_mutex_unlock(&myMute);
}

int main(void){
	pthread_t thread;
	int result_code;
	
	pthread_mutex_lock(&myMute);

	result_code = pthread_create(&thread, NULL, 
				perform_work, &myMute);	
	result_code = pthread_tryjoin_np(thread, NULL);

	pthread_mutex_unlock(&myMute);
	usleep(1000);
	pthread_mutex_lock(&myMute);
	char* path = "./time.txt";
	char* opt = malloc(sizeof(char) * 10);
	opt[0] = 'r';
	int fd = open(path, O_RDONLY);
	FILE* stream = fdopen(fd, opt);
	char word[64] = {'\0'};
	fgets(word, 64, stream);
	printf("%s", word);
	return 0;
}
