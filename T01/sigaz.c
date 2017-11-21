#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//recopier ici bor_signal()
int bor_signal(int sig,void (*h)(int), int options){
	int r; struct sigaction s;
	s.sa_handler=h;sigemptyset (&s.sa_mask); s.sa_flags=options;
	r = sigaction(sig,&s,NULL);
	if(r<0) bor_perror(__func__); /* cf bor-util.c en TP */
	return r;
}

int glo_cpt=10;
void capter(int sig){
	printf("capte %d\n",sig );
	switch(sig){
		case SIGUSR1:
			glo_cpt+=5;break;
		case SIGUSR2:
			glo_cpt-=2;break;
	}
}

int main(){
	pid_t pere=getpid(), p;
	p=fork();

	if (p < 0){
		perror("fork");
		exit(1);
	}

	if(p == 0){			//fils
		printf("je suis %d fils de %d \n", (int) getpid(), pere );
		bor_signal(SIGUSR1, capter,SA_RESTART);
		bor_signal(SIGUSR2, capter,SA_RESTART);
		for(;glo_cpt<0;glo_cpt--){
			printf("%d\n", glo_cpt);
			sleep(1);
		}
		printf("fin du fils\n");
		exit(0);
	}

	else{					//pere
		printf("je suis %d pere de %d \n", pere, p );
		bor_signal(SIGCHLD,capter,SA_RESTART);
		int c = getchar();
		while(c!=EOF){
			switch(c){
				case 0:
					kill(SIGUSR1,p);
					break;
				case 1:
					kill(SIGUSR2,p);
					break;
			}
			c=getchar();
		}
		printf("fin du pere\n");
		exit(0);
	}
}
