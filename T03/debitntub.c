/**
* FILENAME : debitntub.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/
#define _GNU_SOURCE

#include "bor-util.h"
#include "bor-timer.h"
#include <assert.h>

#define TUBMAX 10

size_t char_ecrits;

int maxTab(int tab[][2], int tab_size, int mode){
	int max = -1;
	if (tab_size > 0) 
		max = tab[0][mode];
	for (int i = 0 ; i < tab_size ; i++)
		max = (max < tab[i][mode])?tab[i][mode]:max;
	return max;
}

void ouvrir_tubes (int tubes[][2], int ntubes){
	for (int i = 0; i < ntubes; i++){
		if (pipe(tubes[i]) < 0) {
			perror ("pipe"); exit (1);
		}
	}
}

void fermer_tubes_lecture (int tubes[][2], int ntubes){
	for (int i = 0; i < ntubes; i++) {
		close (tubes[i][0]);
	}
}

void fermer_tubes_ecriture (int tubes[][2], int ntubes){
	for (int i = 0; i < ntubes; i++) {
		close (tubes[i][1]);
	}
}

void dispTotalChar(int sig){
	printf("%zu\n", char_ecrits);
	char_ecrits = 0;
	alarm(1);
}

void fermer (int sig){
	printf("Signal %d recu\n", sig);
	exit (1);
}

void main_fils1 (int tubes[][2], int ntubes, int bufsize){
	printf("début fils %d\n",getpid());
	bor_signal(SIGALRM, dispTotalChar, SA_RESTART);	
	alarm(1);
	fermer_tubes_ecriture (tubes, ntubes);
	char s[bufsize];
	while (1){
		fd_set set_read;
		size_t char_lus;
		FD_ZERO (&set_read);
		for (int i = 0; i < ntubes; i++)
			FD_SET (tubes[i][0], &set_read);
		int res = select (maxTab(tubes, ntubes, 0)+1, &set_read, NULL, NULL, NULL);
		if (res < 0) { perror ("select (fils)"); break; }/
		for (int i = 0; i < ntubes; i++){
			if (FD_ISSET (tubes[i][0], &set_read)) {
				char_lus = bor_read_str(tubes[i][0], s, bufsize);
				if (char_lus <= 0)
					break;
			}
				
		}
		if (char_lus == 0) {
			printf ("fin_pipe\n"); //fin_pipe();
			exit (1);
		}
		char_ecrits += char_lus;
	}
	fermer_tubes_lecture (tubes, ntubes);
	printf("fin fils %d\n",getpid());
	exit (0);
}

void main_pere (int tubes[][2], int ntubes, int bufsize){
	bor_signal(SIGPIPE, fermer, SA_RESTART);
	printf("début père %d\n",getpid());
	fermer_tubes_lecture (tubes, ntubes);
	char s[bufsize];
	char a = 'a';
	while (1){
		fd_set set_write;
		FD_ZERO (&set_write);
		for (int i = 0; i < ntubes; i++)
			FD_SET (tubes[i][1], &set_write);
		int res = select (maxTab(tubes, ntubes, 1)+1, NULL, &set_write, NULL, NULL);
		if (res < 0) { perror ("select (pere)"); break; }
		for (int i = 0; i < ntubes; i++){
			if (FD_ISSET (tubes[i][1], &set_write)){
				int k = bor_write (tubes[i][1], &a, bufsize);
				if (k <= 0) break;
			}
		}
	}
	fermer_tubes_ecriture (tubes, ntubes);
	printf("fin père %d\n",getpid());
	exit (0);
}

int main(int argc, char** argv){
	if(argc!=3){
		fprintf(stderr, "%s buffer_size nb_pipe\n", argv[0]);
		exit(1);
	}
	int bufsize = atoi(argv[1]);
	int ntubes = atoi(argv[2]);
	assert (ntubes <= TUBMAX);
	int tubes[ntubes][2];
	ouvrir_tubes (tubes, ntubes);
	char_ecrits = 0;
	int f = fork ();
	if (f < 0) {
		perror ("fork"); exit (1);
	}	
	if (f == 0){
		main_fils1 (tubes, ntubes, bufsize);
	}
	main_pere (tubes, ntubes, bufsize);	
	exit (0);
}
