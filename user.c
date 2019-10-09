#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <time.h>
#include <semaphore.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
// shared memory structure
struct shmseg{
	unsigned int sec;
	unsigned int nano_sec;
	unsigned timeArray[3];
};

int main(int argc, char* argv[]){
	// random numebr seed
	time_t t;
	srand((unsigned)time(&t));
	
	// starting shared memory
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
	// recieving the values stored in shared memory then add a random number to it
	unsigned int child_sec = shmp->sec;
	unsigned int child_nano_sec = shmp->nano_sec;

	// randomizing child termination time
	child_nano_sec = child_nano_sec+(rand()%(10000000000)+1);
	if(child_nano_sec < 0){
		child_nano_sec *= -1;
	}
	
	// if its over one second, add one to secs .
	while(child_nano_sec >= 1000000000){
		child_sec = child_sec+1;
		child_nano_sec = child_nano_sec -1000000000;
	}

	// inifite loop until the master clock passes the child clock after implementation
	while(1){
		if(child_nano_sec < shmp->nano_sec){
			if(child_sec < shmp->sec || child_sec == shmp->sec){ 
				if(shmp->timeArray[0]==0 && shmp->timeArray[1] ==0 && shmp->timeArray[2]==0){	
					wait(0.05);
					shmp->timeArray[0] = child_sec;
					shmp->timeArray[1] = child_nano_sec;
					shmp->timeArray[2] = getpid();
					break;
				}
			}
		}else{
			if(child_sec < shmp->sec){
				if(shmp->timeArray[0]==0 && shmp->timeArray[1] ==0 && shmp->timeArray[2]==0){
					wait(0.05);
			                shmp->timeArray[0] = child_sec;
                                        shmp->timeArray[1] = child_nano_sec;
                                        shmp->timeArray[2] = getpid();
                                        break;
                                } 
			}
		}	
	}
		
	kill(getpid(),SIGTERM);
	return 0;
}

