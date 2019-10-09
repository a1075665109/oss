#include <stdio.h>
#include <stdlib.h>
// signal master termination time
#include <signal.h>
// getopt
#include <unistd.h>
#include <semaphore.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
pid_t child_pid = -1;
char *logFile = "logFile";
int maxTime = 5;
int tempChild = 5;

// shared memory structure
struct shmseg{
	int sec;
	int nano_sec;
	int timeArray[3];
};

// alarm handler that terminates the parent process
void alarmHandler(int sig){
	FILE *fp;
	fp = fopen(logFile,"a");
	fprintf(fp,"Master process terminated after the the maximum amount of time: %dseconds\n",maxTime);
	fclose(fp);
	if(child_pid!=-1){
                kill(child_pid,SIGTERM);
        }
	printf("Execute success! the results have been stored into file \"logFile\"\n");
        kill(getpid(),SIGTERM);
}

int main(int argc, char* argv[]){
	signal(SIGALRM,alarmHandler);
	FILE *fp;
	// delaration of opt for getopt, default maxChild, command line flag
	int opt;
	int maxTotalChild =95;
	int maxChild = 5;
	int hflag, sflag, lflag,tflag;
	int status = 0;	
	pid_t wpid;
	// a while loop that processes all command line arguments (getopt)
	while((opt = getopt(argc,argv,"hs:l:t:"))!=-1){
                switch(opt){
			case 'h':
				printf("Legal command line options: -h -s -l -t\n\n");
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
	// initializing shared sec and nano sec
	shmp->sec =0;
        shmp->nano_sec =0;
	shmp->timeArray[0] = 0;
	shmp->timeArray[1] = 0;
	shmp->timeArray[2] = 0;
	alarm(maxTime);
	// loop tempChild amount of times to fork child, tempChild default 5 or given as command line arg
	while(tempChild>0){	
		// if it is in the parent
		if(child_pid>0|| child_pid == -1){
			
			//fork and then decrease the amount of child because check condition on while loop
			child_pid = fork();
			tempChild = tempChild -1;
			
			// going inside of child process, execvp to execute ./user
			if(child_pid <=0){
				execvp("./user",NULL);
				// program never reached this part because of execvp
				kill(getpid(),SIGTERM);
			}
		}
	}
	
	// infinite loop that increments the clock(nano_seconds) by 1000
	while(1){
		shmp->nano_sec = shmp->nano_sec +1000;			
		if(shmp->nano_sec >= 1000000000){
			shmp->sec = shmp->sec +1;
			shmp->nano_sec =0;
		}					
	
			// if the message que is not empty, grab the info
			if( shmp->timeArray[2]!=0 ){
				fp = fopen(logFile,"a");
				fprintf(fp,"Master: child pid %d terminated at my time %d.%d because it reached %d.%d  in child process\n",
					shmp->timeArray[2],shmp->sec,shmp->nano_sec,
					shmp->timeArray[0],shmp->timeArray[1]);	
			// clearing the shared memory for critical section comparison
				shmp->timeArray[0] = 0;
                        	shmp->timeArray[1] = 0;
                        	shmp->timeArray[2] = 0;
				fclose(fp);
			// decreament the max amount of child process then fork another child because one is terminated
				if(maxTotalChild >0){	
					maxTotalChild = maxTotalChild-1;
					child_pid=fork();
					if(child_pid <=0){
						execvp("./user",NULL);
						kill(getpid(),SIGTERM);
					}
				}
			}	
		
	}
	while ((wpid = wait(&status)) > 0);
	return 0;
}









