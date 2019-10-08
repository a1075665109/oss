#include <stdio.h>
#include <stdlib.h>
// signal master termination time
#include <signal.h>
// getopt
#include <unistd.h>

pid_t child_pid = -1;
char *logFile = "logFile";
int maxTime = 5;
int tempChild = 5;

struct shared{
	int sec;
	int nano_sec;
	char * shmMsg;
};


void alarmHandler(int sig){
	FILE *fp;
	fp = fopen("logFile","a");
	fprintf(fp,"Master process terminated after the the maximum amount of time: %dseconds\n",maxTime);
	fclose(fp);
	if(child_pid!=-1){
                kill(child_pid,SIGTERM);
        }
        kill(getpid(),SIGTERM);
}

int main(int argc, char* argv[]){
	signal(SIGALRM,alarmHandler);
	int opt;
	int maxChild = 5;
	int hflag, sflag, lflag,tflag;
	int status = 0;	
	pid_t wpid;
	while((opt = getopt(argc,argv,"hs:l:t:"))!=-1){
                switch(opt){
			case 'h':
				hflag = 1;
				break;
		
			case 's':
				sflag = 1;
				maxChild = atoi(optarg);
				tempChild = maxChild;
				break;

			case 'l':
				lflag = 1;
				logFile = optarg;
				break;
	
			case 't':
				tflag = 1;
				maxTime = atoi(optarg);
				break;
			default:
				break;
		}
	}
	// alarm that'll terminate everything after given time has passed;
	alarm(maxTime);

	while(tempChild>0){	
		if(child_pid>0|| child_pid == -1){
			printf("%d \n",tempChild);
			child_pid = fork();
			tempChild = tempChild -1;
			if(child_pid <=0){
				system("./user");
				kill(getpid(),SIGTERM);
			}
		}
	}
	while( (wpid = wait(&status))>0);
	printf("hello\n");
	return 0;
}

