#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
	
#ifdef SINGLE
	int MODE = 1;
#else
	int MODE = 2;
#endif  

#define BUFFER_SIZE 4096
void SigHandler(int signo);
int getLine (char* buffer, int size);
void sendMessage (pid_t pid, char * buffer);
void installSigactions( int, struct sigaction* );

int counter_usr1 = 0;
int counter_usr2 = 0;
int character = 0;
int letter;
int timeSet = 0;
struct timeval start;
struct timeval endtime;
char receivedstr [BUFFER_SIZE];

int main(int argc, char* argv[]) {
	pid_t PID;
	pid_t oPID;
	int end = 0;
	PID = getpid();
	char selfBuffer[BUFFER_SIZE];
	
	struct sigaction act;
	act.sa_handler = SigHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	
	printf("Own PID: %d\n", PID);
	scanf("%d", &oPID);
	
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		printf("sigaction error!\n");
		return 1;
	}
	if (sigaction(SIGUSR2, &act, NULL) < 0) {
		printf("sigaction error!\n");
		return 1;
	}
		
	while(!end) {
		if (getLine(selfBuffer, BUFFER_SIZE) > 0) {
			sendMessage(oPID, selfBuffer);
		}
	}
	
	printf("%d\n", oPID);
	
	return 0;
}

void SigHandler(int signo) {
	if (MODE == 2) {
		if (signo == SIGUSR1) {
			character = (character << 1) + 1;
			++counter_usr1;		//adds 1 bit 
		} else if (signo == SIGUSR2) {
			character = (character << 1);
			++counter_usr2;		//adds 0 bit
		} else {
			printf("Undefined signal number!\n");
			exit(1);
		}
		
	} else if (MODE == 1) {
	  gettimeofday(&endtime, NULL);
		if (signo == SIGUSR1) {
			printf("%ld, timeSet: %d\n", endtime.tv_usec - start.tv_usec, timeSet);
			if(!timeSet){
				timeSet = 1;
				gettimeofday(&start, NULL);
			} else if(endtime.tv_usec - start.tv_usec < 60 && timeSet) {
				timeSet = 0;
				printf("Got 1\n");
				character = (character << 1) + 1;
				++counter_usr1;
			} else if (endtime.tv_usec - start.tv_usec >= 60 && timeSet) {
			  timeSet = 0;
			  printf("Got 0\n");
				character = (character << 1);
				++counter_usr1;
			}
		}	
		
	}
	if (counter_usr1 + counter_usr2 == 8) {
		counter_usr1 = 0;
		counter_usr2 = 0;
		printf("letter at: %d\n", letter);
		receivedstr[letter++] = character;
		
		if(character == 0) {
			printf("%s\n", receivedstr);
			letter = 0;
		}
		character = 0;
	}
	return;
}

int getLine (char* buffer, int size){
  int i;
  for(i = 0;i < size-1;i++){
  	char c = getchar();
  	if(c == '\n' || c == '\r') break;
  	if(c != EOF) {
  		buffer[i] = c;
  	} else {
  		i--;
  	}
  }
  buffer[i] = '\0';
  return i;
}

void sendMessage (pid_t pid, char * buffer) {
	for(int i = 0; buffer[i] != '\0'; i++) {
		printf("current letter: %c\n", buffer[i]); 
		for(int j = 0; j < 8; j++) {
			if(MODE == 2){
				if(buffer[i] & (1 << (7-j))) {
					kill(pid, SIGUSR1);		//sending 0
				} else {
					kill(pid, SIGUSR2);		//sending 1
				}
	
			} else {
				if(buffer[i] & (1 << (7-j))) {
					kill(pid, SIGUSR1);
					sleep(0.05);
					kill(pid, SIGUSR1);
					printf("Sent 1\n");
				} else {
					kill(pid, SIGUSR1);
					sleep(0.5);
					kill(pid, SIGUSR1);
					printf("Sent 0\n");
				}
			}
			sleep(0.05);
		}
	}
	for(int j = 0; j < 8; j++) {
		if (MODE == 2) {
			kill(pid, SIGUSR2);
		} else {
			kill(pid, SIGUSR1);
			sleep(0.2);
			kill(pid, SIGUSR1);
		}
		sleep(0.05);
	}
	return;
}
