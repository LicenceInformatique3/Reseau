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
	if(argc!=3){
		fprintf(stderr, "%s buffer_size nb_pipe\n", argv[0]);
		exit(1);
	}
	pid_pere=(int) getpid();
	int bufsize=atoi(argv[1]);
	int nb_tube=atoi(argv[2]);
	char s[bufsize];					
	
	int * pipes[nb_tube];
	for (int i = 0; i < nb_tube; ++i){
		pipes[i]=malloc(sizeof(int)*2);
		if (pipe(pipes[i]) < 0){
			perror("pipe");
			exit(1);
		}
	}
	
	
	int f=fork();
	if (f<0) {
		perror("fork");
		exit(1);
	}
	if (f==0){							//debut fils
		for (int i = 0; i < nb_tube; ++i){
			close(pipes[i][1]);
		}
		bor_signal(SIGALRM, secend, SA_RESTART);
		pid_fils = (int) getpid();
		alarm(1);
		size_t lu;
		while(1){
			fd_set set_read;
			FD_ZERO(&set_read);
			for (int i = 0; i < nb_tube; ++i){
				FD_SET(pipes[i][0],&set_read);
			}
			int res=select(max()+1,&set_read,NULL,NULL,NULL);				// faire max
			if(res<0){
				perror("select");
				break;
			}
			for (int i = 0; i < nb_tube; ++i){
				if (FD_ISSET(pipes[i][0],&set_read)){
					lu=bor_write(pipes[i][0],s,bufsize);
					if(lu<0){
						break;
					}
				}
			}

			lu=bor_read_str(p[0],s,bufsize);
			if (lu==0){
				fin_pipe();
			}
			octetlu+=lu;
		}

		for (int i = 0; i < nb_tube; ++i){
			close(pipes[i][0]);
		}
		exit(0);
	}									//suite père
	for (int i = 0; i < nb_tube; ++i){
		close(pipes[i][0]);
	}
	bor_signal(SIGPIPE, fin_pipe, SA_RESTART);
	pid_fils=f;
	printf("pere %d , fils %d \n", pid_pere, pid_fils );
	size_t k;
	while(1){
		fd_set set_write;
		FD_ZERO(&set_write);
		for (int i = 0; i < nb_tube; ++i){
			FD_SET(pipes[i][1],&set_write);
		}
		int res=select(max()+1,&set_read,NULL,NULL,NULL);				// faire max
		if(res<0){
			perror("select");
			break;
		}
		for (int i = 0; i < nb_tube; ++i){
			if (FD_ISSET(pipes[i][1],&set_write)){
				k=bor_write(pipes[i][1],s,bufsize);
				if(k<0){
					break;
				}
			}
		}
	}

	for (int i = 0; i < nb_tube; ++i){
		close(pipes[i][1]);
	}
	return 0;
}