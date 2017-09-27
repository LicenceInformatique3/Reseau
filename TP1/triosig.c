#define _GNU_SOURCE
//#include <stdio.h>
//#include <sys/wait.h>
#include "bor-util.h"
#include "bor-timer.h"

int pid_father,pid_child1 ,pid_child2;

void reflectSignal (int sig){
	int current_pid = (int) getpid (); 
	char* prcs_name = (current_pid == pid_father)?"père":((current_pid == pid_child1)?"enfant 1":"enfant 2");
    printf ("%d %s capte %d\n", current_pid,prcs_name , sig); 
    sleep(1);					//pour tester a vitesse réduite
    if (current_pid == pid_father){
        kill (pid_child2, SIGUSR1);
        printf("hey p\n");
    }
    else if (current_pid == pid_child1){
        kill (pid_father, SIGUSR1);
        printf("hey 1\n");
        alarm(5);
    }
    else if (current_pid == pid_child2){
        kill (pid_child1, SIGUSR1);
        printf("hey 2\n");
        alarm(5);
    }
	
}

void timeout(int sig){
    int current_pid = getpid(); 
    if ((current_pid == pid_father) || (current_pid == pid_child1) || (current_pid == pid_child2)){
        char* prcs_name = (current_pid == pid_father)?"père":((current_pid == pid_child1)?"enfant 1":"enfant 2");
        printf("Le processus %s va être fermé (timeout).\n",prcs_name);
        exit(0);
    }
}

void forcestop(int sig){ 														//pour tester l'arret avec CTRL + c
    int current_pid = getpid(); 
    if (current_pid == pid_father){
    	kill (pid_child2, SIGUSR1);
        printf("Le processus pere va être fermé (interupt).\n");
        exit(0);
    }
}

int main(int argc, char * argv[]){
	pid_father = getpid();
	bor_signal(SIGUSR1, reflectSignal, SA_RESTART);
    bor_signal(SIGALRM, timeout, SA_RESTART);
    bor_signal(SIGINT, forcestop, SA_RESTART);
	
	int p1 = fork();
    if (p1 < 0) {
		perror ("Echec fork, fils 1\n");
		exit (1);
	}
	if (p1 == 0){		//Fils1
		pid_child1 = getpid(); 
		alarm(5);
		while(1){
			sleep(1);
		}
		exit(0);
	}					//suite père
	
	pid_child1=p1;
	int p2 = fork();
    if (p2 < 0) {
		perror ("Echec fork, fils 2\n");
		exit (1);
	}
	if (p2 == 0){		//Fils2
		pid_child2 = getpid(); 
		alarm(5); 
		kill(pid_child1, SIGUSR1);
		while(1){
			sleep(1);
		}
		exit(0);
	}					//suite père

	pid_child2=p2; 
	while(1){
		sleep(1);
	}
	return 0;
}
