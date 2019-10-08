#include <stdio.h>
#include <stdlib.h>
// signal master termination time
#include <signal.h>
// getopt
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
pid_t child_pid = -1;
char *logFile = "logFile";
int maxTime = 5;
int tempChild = 5;

struct shmseg{
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
	
	// shared memory segment
	int shmid;
	struct shmseg *shmp;
	shmid = shmget(0x1234,sizeof(struct shmseg),0755|IPC_CREAT);
	if(shmid == -1){
		perror("Shared memory\n");
		return 0;
	}

	shmp = shmat(shmid,NULL,0);
	if(shmp == (void *)-1){
		perror("Shared memory attach\n");
		return 0;
	}
	shmp->sec =0;
        shmp->nano_sec =0;
	shmp->shmMsg =0;
	
	int a =2;	

	while(tempChild>0){	
		a+=1;
		if(child_pid>0|| child_pid == -1){
			printf("%d \n",tempChild);
			child_pid = fork();
			tempChild = tempChild -1;
			if(child_pid <=0){
				sleep(a);
				execvp("./user",NULL);
				printf("hello\n");
				kill(getpid(),SIGTERM);
			}
		}
	}
	
	while(1){
		sleep(1);
			shmp->nano_sec = shmp->nano_sec +1000;
			if(shmp->nano_sec >= 1000000000){
				shmp->sec = shmp->sec +1;
				shmp->nano_sec -= 1000000000;
			}
	//		printf("Seconds: %d, Nano-seconds: %d\n",shmp->sec,shmp->nano_sec);
		
	}
	return 0;
}

