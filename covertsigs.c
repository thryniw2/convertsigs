#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
	
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
void waiting (long nsec);

pid_t oPID;
int counter_usr1 = 0;
int counter_usr2 = 0;
int character = 0;
int letter;
int sender_flag = 0;
int timeSet = 0;
int receiverSet = 1;
int zero = 0;
struct timespec start;
struct timespec endtime;
char receivedstr [BUFFER_SIZE];

int main(int argc, char* argv[]) {
	pid_t PID;
	//int end = 0;
	PID = getpid();
	char selfBuffer[BUFFER_SIZE];
	
	struct sigaction act;
	act.sa_handler = SigHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	signal(SIGALRM, act.sa_handler);
	
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
		
	while(1) {
		if(!timeSet) {
			if (getLine(selfBuffer, BUFFER_SIZE) > 0) {
				sendMessage(oPID, selfBuffer);
			}	
		} else {
			kill(oPID, SIGUSR1);
			waiting(500000);
			if(timeSet) {
				zero = 1;
			}
			raise(SIGUSR1);
		}
	}
	
	return 0;
}

void waiting (long nsec){
	clock_gettime(CLOCK_MONOTONIC, &start);
	endtime.tv_nsec = start.tv_nsec + nsec;
	while (start.tv_nsec < endtime.tv_nsec) {
		//printf("%ld\n", endtime.tv_nsec - start.tv_nsec);
		sleep(0.05);
		clock_gettime(CLOCK_MONOTONIC, &start);
	}
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
		if (signo == SIGUSR1 && !sender_flag) {
			if (timeSet && !zero) {
				character = (character << 1) + 1;
				++counter_usr1;
				kill(oPID, SIGUSR1);
			} else if (timeSet && zero) {
				character = (character << 1);
				++counter_usr1;
				zero = 0;
				kill(oPID, SIGUSR1);
			} else {
				timeSet = 1;
				return;
			}
		} else if (signo == SIGUSR1 && sender_flag) {
			receiverSet = 0;
		}
	}
	if (counter_usr1 + counter_usr2 >= 8) {
		counter_usr1 = 0;
		counter_usr2 = 0;
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
	if (MODE == 1) {
		printf("sender change\n");
		sender_flag = 1;
		kill(pid, SIGUSR1);		//Start single signal message
		sleep(0.01);
	}
	for(int i = 0; buffer[i] != '\0'; i++) { 
		for(int j = 0; j < 8; j++) {
			if(MODE == 2){
				if(buffer[i] & (1 << (7-j))) {
					kill(pid, SIGUSR1);		//sending 0
				} else {
					kill(pid, SIGUSR2);		//sending 1
				}
	
			} else {
				if(buffer[i] & (1 << (7-j))) {
					while(receiverSet){ sleep(0.01); }
					receiverSet = 1;
					printf("Sent 1\n");
					sleep(0.01);
					kill(pid, SIGUSR1);		//sending 1
				} else {
					while(receiverSet){ sleep(0.01); }
					receiverSet = 1;
					printf("Sent 0\n");		//sending 0
				}
				sleep(0.01);
			}
			
		}
		if (MODE == 1 && buffer[i] == 0) {
			kill(pid, SIGUSR1);	//end of letter.
		}
	}
	for(int j = 0; j < 8; j++) {
		if (MODE == 2) {
			kill(pid, SIGUSR2);
			sleep(0.05);
		} else {
			while(timeSet){ sleep(0.01); }//long wait to symbolize message end.
			timeSet = 1;
		}
	}
	sender_flag = 0;
	return;
}
