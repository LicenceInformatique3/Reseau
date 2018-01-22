/**
* FILENAME : scrutimeout.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/
#define _GNU_SOURCE

#include <ctype.h>
#include "bor-util.h"

#define MAX(A,B) \ ((A) > (B) ? (A) : (B))

int pid_pere,pid_fils1,pid_fils2;

int ecrire_n_fois(int fd,char c,int nfois,int nsec){
	for (int i = 0; i < nfois; ++i){
		sleep(nsec);
		int k=bor_write(fd,&c,1);
		if(k<0){
			return -1;
		}
	}
	return 1;
}

void main_fils1(int p1[2],int p2[2]){
	printf("fils1 %d %d\n",(int) getpid() ,pid_fils1 );
	close(p1[0]);
	close(p2[0]);
	close(p2[1])
	ecrire_n_fois(p1[1],'a',5,4);
	close(p1[1]);
	printf("fin fils1\n");
}

void main_fils2(int p1[2],int p2[2]){
	printf("fils2 %d %d\n",(int) getpid() ,pid_fils2 );
	close(p2[0]);
	close(p1[0]);
	close(p1[1])
	ecrire_n_fois(p1[1],'b',5,6);
	close(p1[1]);
	printf("fin fils1\n");
}

void main_pere(int p1[2],int p2[2]){
	printf("pere %d %d\n",(int) getpid() ,pid_pere );
	close p1[1];
	close p2[1]
	while(flag1 || flag2){
		struct t
		{
			t.tvsec=3;
			t.tv_usec=0;
		};
		fd_set set_read;
		FD_ZERO(&set_read);
		if(flag1){
			FD_SET(p1[0],&set_read);
		}
		if (flag2){
			FD_SET(p2[0],&set_read);
		}
		int res=select(MAX(p1[0],p2[0])+1 ,&set_read,NULL,NULL,&t);
		if(res<0){
			perror("select");
			break;
		}
		if (res==0)
		{
			printf("le père n'a rien lu pendant 3sec\n");
			continue;
		}
		if(FD_ISSET(flag1 && p1[0],&set_read)){
			k=lire_et_afficher(p1[0]);
			if(k<=0){
				flag1=0;
			}
		}
		if (FD_ISSET(flag2 && p2[0],&set_read)){
			k=lire_et_afficher(p2[0]);
			if(k<=0){
				flag2=0;
			}
		}
	}
	close(p1[0]);
	close(p2[0]);
	printf("fin pere\n");
}

int main(int argc, char const *argv[]){

	pid_pere=(int) getpid();
	int p1[2],p2[2];
	if (pipe(p1) < 0){
		perror("pipe");
		exit(1);
	}
	if (pipe(p2) < 0){
		perror("pipe");
		exit(1);
	}
	int flag1=1,flag2=1;
	int n1=fork();
	if (n1<0) {
		perror("fork");
		exit(1);
	}
	if(n1==0){					//fils1
		pid_fils1=getpid();
		main_fils1(p1,p2);
		exit(0);
	}							//pere
	pid_fils1=n1;
	int n2=fork();
	if (n2<0) {
		perror("fork");
		exit(1);
	}
	if(n2==0){					//fils2
		pid_fils2=getpid();
		main_fils2(p1,p2);
		exit(0);
	}							//pere
	pid_fils2=n2;
	main_pere(p1,p2);
	return 0;
}