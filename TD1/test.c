#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){

	pid_t p=getpid();
	printf("%d\n",p );
	fork();
	fork();
	fork();
	printf("slt ! %d \n", (int) getpid());

	if (getpid() == p)
		sleep(1);
	
}