#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "bor-util.h"

int pid_pere,pid_fils;
long long int octetlu;
void secend(){
	if((int) getpid() == pid_fils){
		alarm(1);
		printf("octetlu : %lld\n",octetlu);
		octetlu=0;
	}
	else {										// useless ?
		perror("wrong pid or  process");
		exit(1);
	}
}

void fin_pipe(){
	printf("allo\n");
	if((int) getpid() == pid_pere){
		printf("le fils %d a fermer le pipe / est mort\n",pid_fils);
		exit(0);
	}
	else if((int) getpid() == pid_fils){
		printf("le pere %d a fermer le pipe / est mort\n",pid_pere);
		exit(0);
	}
	else {										// useless ?
		perror("wrong pid or  process");
		exit(1);
	}
}

int main(int argc , char  * argv[]){
	if(argc!=2){
		fprintf(stderr, "%s buffer_size\n", argv[0]);
		exit(1);
	}
	pid_pere=(int) getpid();
	int bufsize=atoi(argv[1]);
	char s[bufsize];					
	
	int p[2];
	if (pipe(p) < 0){
		perror("pipe");
		exit(1);
	}
	int f=fork();
	if (f<0) {
		perror("fork");
		exit(1);
	}
	if (f==0){							//debut fils
		close(p[1]);
		bor_signal(SIGALRM, secend, SA_RESTART);
		pid_fils = (int) getpid();
		alarm(1);
		size_t lu;
		while(1){
			lu=bor_read_str(p[0],s,bufsize);
			if (lu==0){
				fin_pipe();
			}
			octetlu+=lu;
		}

		close(p[0]);
		exit(0);
	}									//suite père
	close(p[0]);
	bor_signal(SIGPIPE, fin_pipe, SA_RESTART);
	pid_fils=f;
	printf("pere %d , fils %d \n", pid_pere, pid_fils );
	while(1){
		bor_write(p[1],s,bufsize);
	}

	close(p[1]);
	return 0;
}