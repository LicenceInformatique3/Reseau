/**
* FILENAME : scrutatub.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/
#define _GNU_SOURCE

#include <ctype.h>
#include "bor-util.h"

int pid_pere,pid_fils;

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
	
/*													//un par un 
int lire_et_afficher(int fd){
	char c;										
	int k=bor_read(fd,&c,1);
	if(k <= 0) {
		return k;
	}
	printf("lu sur %d : %c\n",fd,c);
}
*/

int lire_et_afficher(int fd){
	char buf[100];										
	int k=bor_read_str(fd,buf,sizeof(buf));
	if(k <= 0) {
		return k;
	}
	printf("lu sur %d : %s\n",fd,buf);
}

void main_fils1(int p1[2]){
	printf("fils1 %d %d\n",(int) getpid() ,pid_fils );
	close(p1[0]);
	ecrire_n_fois(p1[1],'a',30,1);
	close(p1[1]);
	printf("fin fils1\n");
}

void main_pere(int p1[2]){
	printf("pere %d %d\n",(int) getpid() ,pid_pere );
	close p1[1];
	while(1){
		fd_set set_read;
		FD_ZERO(&set_read);
		FD_SET(0,&set_read);
		FD_SET(p1[0],&set_read);
		int res=select(p1[0]+1,&set_read,NULL,NULL,NULL);			//p1[0]+1= max(p1[0],0)+1
		if(res<0){
			perror("select");
			break;
		}
		if(FD_ISSET(0,&set_read)){
			k=lire_et_afficher(0);
			if(k<0){
				break;
			}
		}
		if (FD_ISSET(p1[0],&set_read)){
			k=lire_et_afficher(p1[0]);
			if(k<0){
				break;
			}
		}
	}
	close(p1[0]);
	printf("fin pere\n");
}

int main(int argc, char const *argv[]){

	pid_pere=(int) getpid();
	int p1[2];
	if (pipe(p1) < 0){
		perror("pipe");
		exit(1);
	}
	int f=fork();
	if (f<0) {
		perror("fork");
		exit(1);
	}
	if(f==0){					//fils
		pid_fils=getpid();
		main_fils1(p1);
		exit(0);
	}							//pere
	pid_fils=f;
	main_pere(p1);
	return 0;
}