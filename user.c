#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <time.h>

#include <sys/ipc.h>
#include <sys/shm.h>

struct shmseg{
	int sec;
	int nano_sec;
	char *shmMsg;
};

int main(int argc, char* argv[]){
	srand(time(0));
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
	
	int child_sec = shmp->sec;
	int child_nano_sec = shmp->nano_sec +(rand()%(10000)+1);
	if(child_nano_sec >= 1000000000){
		child_sec += 1;
		child_nano_sec -=1000000000;
	}
	while(1){
		if(child_nano_sec < shmp->nano_sec){
			if(child_sec < shmp->sec || child_sec == shmp->sec){ 
				printf("%d, %d",shmp->sec,shmp->nano_sec);
				break;
			}
		}	
	}
		
	pid_t me = getpid();
	printf("My pid is %d\n",me);
	kill(getpid(),SIGTERM);
	return 0;
}

