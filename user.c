#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
int main(int argc, char* argv[]){
	pid_t me = getpid();
	printf("My pid is %d\n",me);
	kill(getpid(),SIGTERM);
	return 0;
}

