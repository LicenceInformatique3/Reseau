/**
* FILENAME : heritub.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/
#define _GNU_SOURCE

#include <ctype.h>
#include "bor-util.h"

int main(){
	char buf[100];
	int p1[2];
	if (pipe(p1) < 0){
		perror("pipe");
		exit(1);
	}
	bor_read_str(0,buf,sizeof(buf));
	bor_write(p1[1],buf,10);
	bor_read_str(p1[0],buf,4);
	bor_write_str(1,buf);
	printf("\n");

	int f=fork();
	if (f<0) {
		perror("fork");
		exit(1);
	}
	if (f==0){						//debut fils
		close(p1[1]);
		bor_read_str(p1[0],buf,8);
		bor_write_str(1,buf);
		printf("\n");	
		close(p1[0]);
		exit(0);
	}								//suite père

	close(p1[1]);
	close(p1[0]);
	wait(NULL);
	return 0;
}