#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef SINGLE
	const int MODE = 2;
#endif  

#define BUFFER_SIZE 4096
void SigHandler(int signo);
int getLine (char** buffer, int size, FILE* stream);
void sendMessage (pid_t pid, char * buffer);
void installSigactions( int, struct sigaction* );

int counter_usr1 = 0;
int counter_usr2 = 0;
int character = 0;
char * receivedstr;

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
		if (getLine(&selfBuffer, BUFFER_SIZE, stdin) > 0) {
			sendMessage(oPID, selfBuffer);
		}
	}
	
	
	printf("%d\n", oPID);
	
	return 0;
}

void SigHandler(int signo) {
	char chr[1];
	if (signo == SIGUSR1) {
		character = (character << 1);
		++counter_usr1;
	} else if (signo == SIGUSR2) {
		character = (character << 1) + 1;
		++counter_usr2;
	} else {
		printf("Undefined signal number!\n");
		exit(1);
	}
	if (counter_usr1 + counter_usr2 == 8) {
		counter_usr1 = 0;
		counter_usr2 = 0;
		itoa(character, chr, 10);
		strcat(receivedstr, chr);
		if(character == 0) {
			printf("%s", receivedstr);
		}
		character = 0;
	}
	return;
}

int getLine (char** buffer, int size, FILE* stream){
  int i;
  for(i = 0;i < size-1;i++){
  	char c = getc(stream);
  	if(c == '\n' || c == '\r') break;
  	buffer[i] = &c;
  }
  for (int j = i; j < size; j++) buffer[j] = '\0';
  return i;
}

void sendMessage (pid_t pid, char * buffer) {
	for(int i; buffer[i] != '\0'; i++) {
		for(int j = 0; j < 8; j++) {
			if(buffer[i] & (1 << j)) {
				kill(pid, SIGUSR1);
			} else {
				kill(pid, SIGUSR2);
			}
			sleep(0.05);
		}
	}
	for(int j = 0; j < 8; j++) {
		kill(pid, SIGUSR1);
	}
	return;
}
