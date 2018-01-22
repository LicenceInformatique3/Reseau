/**
* FILENAME : tradu2tub.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/
#define _GNU_SOURCE

#include "bor-util.h"

void capterSigpipe (int sig){
	printf("Signal %d recu\n", sig);
	exit (1);
}

void main_fils1 (int p1[2], int p2[2]){
	printf("début fils %d\n",getpid());
	
	close(p1[1]);
	close(p2[0]);
	
	close(0);
	dup(p1[0]);
	close(p1[0]);
	
	close(1);
	dup(p2[1]);
	close(p2[1]);
	
	execlp("/usr/bin/stdbuf", "stdbuf", "-i0", "-o0", "-e0", "tr", "a-z", "A-Z", NULL);
	perror("stdbuf tr");
	
	exit(1);
	
}

void main_pere (int p1[2], int p2[2]){
	bor_signal(SIGPIPE, capterSigpipe, SA_RESTART);
	
	printf("début père %d\n",getpid());
	
	close(p1[0]);
	close(p2[1]);
	
	while (1){
		fd_set set_read;
		
		FD_ZERO (&set_read);
		FD_SET (0, &set_read);
		FD_SET (p2[0], &set_read);
		
		int res = select (p2[0]+1, &set_read, NULL, NULL, NULL);
		if (res < 0) { perror ("select (pere)"); break; }
		
		if (FD_ISSET (0, &set_read)){
			// recopie dans le tube1
			char buf[100];
			int k = bor_read_str (0, buf, sizeof(buf));
			if (k <= 0) break;
			
			k = bor_write_str (p1[1], buf);
			if (k <= 0) break;
		}
		
		if (FD_ISSET (p2[0], &set_read)){
			// recopie dans la sortie standard
			char buf[100];
			int k = bor_read_str (p2[0], buf, sizeof(buf));
			if (k <= 0) break;
			
			k = bor_write_str (1, buf);
			if (k <= 0) break;
		}
	}	
	close(p1[1]);
	close(p2[0]);
}

int main(){
	int p1[2];
	if (pipe(p1) < 0) {
		perror("pipe");
		exit(1);
	}
	
	int p2[2];
	if (pipe(p2) < 0) {
		perror("pipe");
		exit(1);
	}
	
	int f1 = fork();
	if (f1 < 0){
		perror("fork");
		exit(1);
	}
	if (f1 == 0){ // fils 1
		main_fils1 (p1, p2);
	}
	main_pere (p1, p2);
	exit (0);
}
