//gcc -Wall -std=c99 fintub.c bor-util.c -o fintub
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "bor-util.h"
/*

ssize_t bor_read (int fd, void *buf, size_t count){
	ssize_t r=read(fd,buf,count);
	if(r<0){
		perror(__func__);
		return -1;
	}
	if (r==0 && count>0 ){
		printf("fin du fichier %d attente \n",fd );
	}
	return r;
}
ssize_t bor_read_str (int fd, char *buf, size_t count){
	if(count ==0){
		fprintf(stderr, "Erreur dans %s : count <0 attendu \n",__func__);
		return -1;
	}
	ssize_t r=read(fd,buf,count-1);
	if(r>=0){
		buf[r]='\0';
	}
	return r;

}

ssize_t bor_write(int fd, const void *buf, size_t count){
	ssize_t r=write(fd,buf,count);
	if(r<0){
		perror(__func__);
		return -1;
	}
	return r;
}

ssize_t bor_write_str(int fd, const void *buf){
	ssize_t r=bor_write(fd,buf,strlen(buf));
	return r;
}

*/



void mettre_en_majuscules(char *buf){
	for(int i=0;buf[i];i++){
		buf[i]=toupper(buf[i]);
	}
}

int main(){
	char buf[100];
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
	if (f==0){ 					//fils lecteur
		close(p1[1]);
		printf("voici les caractères lus par le fils %d mais en majuscule\n",(int) getpid() );
		while(1){
			int k=bor_read_str(p1[0],buf,sizeof(buf));
			if(k<=0){
				break;
			}
			mettre_en_majuscules(buf);
			k=bor_write_str(1,buf);
			if(k<0){
				break;
			}
		}

		close(p1[0]);
		exit(0);
	}							//suite pere
	close(p1[0]);
	while (1){
		int k= bor_read_str(0,buf,sizeof(buf));  		// ou puts(buf)
		if(k<=0){										//
			break;										//
		}												//
		k=bor_write_str(p1[1],buf);
		if(k<0){
			break;
		}
	}

	close(p1[1]);
	exit(0);
}



/*
*			fils				père
*			ls 			| 		wc -l
*				p[1] 		p[0]
*		ls: 1 --> p[1]
*		wc  0 --> p[0]
*
* redirtube.c
*	- pipe(p1)
*	- fork
*	o fils: écrivan
*		close(p1[0])
*
*		[close (1)
*		[dup(p1[1])
*		[close(p1[1])
*		execlp("ls","ls",NULL);
*		perror("exec ls");
*		exit(1);
*
*
*	o pere: lecteur
*		close(p1[1])
*
*		[close (0)
*		[dup(p1[0])
*		[close(p1[0])
*		execlp("wc","wc","-l",NULL);
*		perror("exec wc");
*		exit(1);
*/