#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void create_child(){
    int p = fork();
    if (p < 0) {
		perror ("fork");
		exit (1);
	}
    if (p == 0){
        for (int i = 10; i > 0; i--){
            printf("%d\n", i);
            sleep (1);
        }
        exit (0);
	}
}

void wait_child (int nb_child){
    for (int i = 0; i < nb_child; i++)
        wait (NULL);
}

int main(int argc, char * argv[]){
	int n, total_child = 0;
	do{
		scanf ("%d", &n);
		total_child += n;
		for(int i = 0; i < n;i++){
			create_child();
		}
	}while(n != 0);
	wait_child(total_child);
	printf("fin de tous les fils détectée\n");
	return 0;
}