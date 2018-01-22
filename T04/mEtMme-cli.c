/**
* FILENAME : mEtMme-cli.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include "bor-util.h"

typedef struct{
	char *tub_ec_nom,
		 tub_cs_nom[100],
		 tub_sc_nom[100];
	int tub_ec, tub_cs, tub_sc;
}Client;

int creer_tubes_service(Client *c){
	printf("Création tubes service\n");
	sprintf(c->tub_cs_nom,"tub_cs_%d",(int)getpid());
	int k = mkfifo(c->tub_cs_nom, 0600);
	if(k < 0){
		perror("mkfifo cs");
		return -1;
	}
	
	sprintf(c->tub_sc_nom,"tub_sc_%d",(int)getpid());
	int k = mkfifo(c->tub_sc_nom, 0600);
	if(k < 0){
		perror("mkfifo sc");
		unlink(c->tub_cs_nom);
		return -1;
	}

	return 0;
}

void supprimer_tubes_service(Client *c){
	printf("Suppression des tubes de service\n");
	unlink(c->tub_cs_nom);
	unlink(c->tub_sc_nom);
}

int ouvrir_tubes_service(Client *c){
	//Ouverture bloquante !
	printf("Ouverture tub_cs...\n");
	c->tub_cs = open(c->tub_cs_nom, 0_WRONLY);
	if(c->tub_cs < 0){
		perror("open tub_cs");
		return -1;
	}
	printf("Ouverture tub_sc...\n");
	c->tub_sc = open(c->tub_sc_nom, 0_RDONLY);
	if(c->tub_sc < 0){
		perror("open tub_sc");
		return -1;
	}
	return 0;
}

void fermer_tubes_service(Client *c){
	printf("Fermeture tubes services\n");
	close(c->tub_cs);
	close(c->tub_sc);
}

int prendre_contact(Client *c){
	printf("Ouverture tub_ec...\n");
	if(open(c->tub_ec_nom, O_WRONLY) < 0){
		perror("open tub_ec");
		return -1;
	}
	printf("Envoi noms tubes \n");
	char buf[1000];
	sprintf(buf, "%s %s", c->tub_cs_nom, c->tub_sc_nom);
	int k = bor_write_str(c->tub_ec, buf);
	close(c->tub_ec);
	return k < 0 ? -1 : 0;
}

int faire_dialogue(Client *c){
	int k;
	char buf[1000];
	
	printf("Entrer un nom de famille : \n");
	k = bor_read_str(0, buf, sizeof(buf));
	if(k <= 0) return k;

	printf("Envoi de %s au serveur\n", buf);
	k = bor_write(c->tub_cs, buf);
	if(k <= 0) return k;

	printf("Attente reponse serveur...\n");
	k = bor_read_str(c->tub_sc, buf,sizeof(buf));
	if(k <= 0) return k;

	printf("Reponse = %s\n", buf);
	return 1;
}

int main( int argc , char *argv[]){
	int r = -1;
	Client c;
	if(argc - 1 != 1){
		fprintf(stderr, "Usage : %s tub_ecoute\n ", argv[0]);
		exit(1);
	}

	c.tub_ec_nom = argv[1];
	//On se protege de SIGPIPE
	bor_signal(SIGPIPE, SIG_IGN, SA_RESTART);
	if(creer_tubes_service(&c) < 0) exit(1);
	if(prendre_contact(&c) < 0) goto fin1;
	if(ouvrir_tubes_service(&c) < 0 ) goto fin1;

	while(1){
		r = faire_dialogue(&c);
		if(r <= 0) break;
	}
	fermer_tubes_service(&c);

	fin1 : 
		supprimer_tubes_service(&c);
	exit(r < 0 ? 1 : 0);
}