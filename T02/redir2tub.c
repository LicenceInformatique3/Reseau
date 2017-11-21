#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "bor-util.h"

int pid_pere,pid_fils1,pid_fils2;

int main(){
	int p1[2];
	if (pipe(p1) < 0){
		perror("pipe");
		exit(1);
	}
	int p2[2];
	if (pipe(p2) < 0){
		perror("pipe");
		exit(1);
	}
	pid_pere=getpid();
	int f1=fork();
	if (f1<0) {
		perror("fork");
		exit(1);
	}
	if (f1==0){						//debut fils 1
		pid_fils1=getpid();
		close(p1[0]);		//on ferme les pipe useless
		close(p2[1]);		
		close(p2[0]);		
		close (1);			// on ferme la sortie standar
		dup(p1[1]);			
		close(p1[1]);
		execlp("ls","ls","-t",NULL);			//recouvrement
		perror("exec ls -t");
		exit(1);
	}								//suite père
	pid_fils1=f1;

	int f2=fork();
	if (f2<0) {
		perror("fork");
		exit(1);
	}
	if (f2==0){						//debut fils 2
		pid_fils2=getpid();
		close(p1[1]);				
		close(p2[0]);
		close(0);					
		dup(p1[0]);
		close(p1[0]);
		close(1);
		dup(p2[1]);
		close(p2[1]);
		execlp("sort","sort",NULL);
		perror("exec sort");
		exit(1);
	}								//suite père 
	pid_fils2=f2;
	close(p1[1]);
	close(p1[0]);
	close(p2[1]);
	close(0);
	dup(p2[0]);
	close(p2[0]);
	execlp("head","head","-3",NULL);
	perror("exec head -3");
	exit(1);

	return 0;
}