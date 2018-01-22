/**
* FILENAME : mEtMme-ser.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#include "bor-util.h"

typedef struct{
	char *fnofam_nom,
		*tub_ec_nom,
		tub_cs_nom[100],
		tub_sc_nom[100];
	int tub_ec, tub_cs, tub_sc;
}Serveur;

int boucle_princ = 1;

void capter_fin(int sig){
	boucle_princ = 0;
}

void capter_fin_fils(int sig){
	printf("Signal %d capté\n", sig);
	wait(-1,NULL,WNOHANG);
}

int creer_tube_ecoute(Serveur *s){

	printf("Creation tube ecoute\n");
	sprintf(s->tub_ec_nom,"tub_ec_%d",(int)getpid());
	int k = mkfifo(s->tub_ec_nom, 0600);
	if(k < 0){
		perror("mkfifo ec");
		return -1;
	}
	return 0;
}

void supprimer_tubes_ecoute(Serveur *s){
	printf("Suppression des tubes d'ecoute\n");
	unlink(s->tub_ec_nom);
 //   close(s->tub_ec);
}

int ouverture_tubes_service(Serveur *s){
	//Ouverture bloquante !
	printf("Ouverture tub_sc...\n");
	s->tub_sc = open(s->tub_sc_nom, 0_WRONLY);
	if(s->tub_sc < 0){
		perror("open tub_sc");
		return -1;
	}

	printf("Ouverture tub_cs...\n");  
	s->tub_cs = open(s->tub_cs_nom, 0_RDONLY);
	if(s->tub_cs < 0){
		perror("open tub_cs");
		return -1;
	}

	return 0;
}

void fermer_tubes_service(Serveur *s){
	printf("Fermeture tubes services\n");
	close(s->tub_cs);
	close(s->tub_sc);
}

void attendre_contact(Serveur *s){
	ouverture_tube_ecoute(*s);
	fscanf(s->tub_ec, "%s %s", s->tub_cs_nom, s->tub_sc_nom);
} 

char* recherche_nom(Serveur *s, char *nom){

	FILE *fichierText = fopen(s->fnofam_nom, "r");
	char resultat[100];
	int nom_trouve = 0;
	if (fichierText == NULL){
		fprintf(stderr,"Impossible d'ouvrir le fichier %s\n",nomFichier);
	}

	if(nom_trouve == 0) sprintf(resultat,"Non Trouvé");

	return resultat;
}

void faire_dialogue(Serveur *s){
	//lecture tub_cs
	char nom[1000], resultat[100];

	printf("Lecture dans tub_cs\n");	
	k = bor_read_str(s->tub_cs, nom, sizeof(nom));
	if(k <= 0) return k;

	resultat = recherche_nom(*s, nom);
	
	printf("Envoi de %s au client\n", resultat);
	k = bor_write(c->tub_sc, resultat);
	if(k <= 0) return k;

}

void fils_main(Serveur *s){
	ouvrir_tubes_service(*s);
	while(boucle_princ){
		faire_dialogue();
	}
	fermer_tubes_service(*s);
}

int faire_transaction(Serveur *s){
	attendre_contact();
	int fils = fork();
	if(fils < 0){ perror("fork"); return -1; }
	if(fils == 0){
		printf("Fermeture tube ecoute\n");
		close(s->tub_ec);
		fils_main(s);
	}

	char buf[10];
	while(bor_read_str(s->tub_ec,buf,sizeof(buf)) > 0);
	close(s->tub_ec);

	return 1;
}

int main(int argc , char *argv[]){

	Serveur s;

	if(argc - 1 != 2){
		fprintf(stderr, "Usage : %s tub_ecoute fichier_noms\n ", argv[0]);
		exit(1);
	}

	s.tub_ec_nom = argv[1];
	s.fnofam_nom = argv[2]

	if(creer_tube_ecoute(&s) < 0) exit(1);

	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART);
	bor_signal(SIGINT,capter_fin,0);
	bor_signal(SIGCHLD,capter_fin_fils,SA_RESTART);
	
	while(boucle_princ){
		r = faire_transaction(&s);
		if(r <= 0) break;
	}
	supprimer_tubes_ecoute(&s);
	exit(0);
}