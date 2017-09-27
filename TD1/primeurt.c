#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc ,char * argv[]){
	if(argc-1 !=2){
		fprintf(stderr ,"usage : %s a b\n", argv[0]);
		exit(1);
	}
	int a=atoi(argv[1]), b=atoi(argv[2]);	

	pid_t pere=getpid(), fils;
	fils=fork();

	if (fils < 0){
		perror("fork");
		exit(1);
	}

	if(fils == 0){			//fils
		printf("je suis %d fils de %d \n", (int) getpid(), pere );
		sleep(b);
		printf("mort du fils\n");
		exit(0);
	}

	else{					//pere
		printf("je suis %d pere de %d \n", pere, fils );
		sleep(a);
		printf("mort du pere\n");
		exit(0);
	}

}



