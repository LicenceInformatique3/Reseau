#include "bor-util.h"

typedef struct {
	char * tub_ec_nom,
	tub_cs_nom[100],
	tub_sc_nom[100];
	int tub_ec,tub_sc,tub_cs;
}Client;

int creer_tubes_service(Client *c){
	printf("Création tubes service\n");
	sprintf(c->tub_cs_nom,"tub_cs_%d",(int)getpid());
	int k=mkfifo(c->tub_cs_nom, 0600);
	if (k<0){
		perror("wkfifo cs");
		return -1;
	}

	sprintf(c->tub_sc_nom,"tub_sc_%d",(int)getpid());
	k=mkfifo(c->tub_sc_nom, 0600);
	if (k<0){
		perror("wkfifo sc");
		unlink(c->tub_cs_nom);
		return -1;
	}
	return 0
}

void supprimer_tubes_service(Client *c){
	printf("Suppresion tubes service\n");
	unlink(c->tub_cs_nom);
	unlink(c->tub_sc_nom);
}

int ouvrir_tubes_service(Client *c){
	//ouvertures bloquantes !!
	printf("ouvertures tub_cs service\n");
	c->tub_cs=open(c->tub_cs_nom,O_WRONLY);
	if (c->tub_cs<0){
		perror("open tub_cs");
		return -1;
	}
	printf("ouvertures tub_cs service\n");
	c->tub_sc=open(c->tub_sc_nom,O_RDONLY);
	if (c->tub_sc<0){
		perror("open tub_sc");
		close(c->tub_cs);
		return -1;
	}
	return 0;
}

/** Tester : ouvrir 2 terminaux
 * / cat < tub_cs_567
 * ~~ il vous répond là
 * \ cat > tub_sc_567
 * tapez ici
 */

void fermer_tubes_service(Client *c){
	printf("Fermetures tub_cs tub_sc service\n");
	close(c->tub_cs);
	close(c->tub_sc);

}

int prendre_contact(Client *c){
	printf("Ouverture tub_ec \n");
	if (open(c->tub_ec_nom,O_WRONLY) <0){
		perror ("open tub_ec")
		return-1;
	}
	printf("Envoi nomstubes\n");
	char buf[1000];
	sprintf(buf,"%s %s",c->tub_cs_nom,c->tub_sc_nom);
	int k=bor_write_str(c->tub_ec,buf);
	close(c->tub_ec);
	return k<0? -1:0;
}

int faire_dialogue(Client *c){
	int k;
	char buf[1000];
	printf("Entrez un nom de famille\n");
	k=bor_read_str(0,buf,sizeof(buf));
	if (k<=0){
		return k;
	}
	printf("Envoi de %s au serveur\n",buf );
	k=bor_write_str(c->tub_cs,buf);
	if(k<0){
		return k;
	}
	printf("Attente réponse serveur ...\n");
	k=bor_read_str(c->tub_sc,buf,sizeof(buf));
	printf("Réponse :%s\n",buf );
	return 1;

}

int main(int argc,char * argv[]){
	Client c;
	int r=-1;
	if (argc-1 != 1){
		fprintf(stderr, "Usage: %s tub_ec\n", argv[0] );
		exit(1);
	}
	c.tub_ec_nom=argv[1];
	bor_signal(SIGPIPE,SIG_ING,SA_RESTART);		//protection contre SIGPIPE
	if (creer_tubes_service(&c) < 0){
		exit(1);
	}
	if(prendre_contact (&c) < 0){
		goto fin1;
	}
	if(ouvrir_tubes_service(&c) < 0){
		goto fin1;
	}
	while(1){
		r=faire_dialogue(&c);
		if(r==0){
			break;
		}
	}


	fin1:
		supprimer_tubes_service(&c);
	exit(r<0 ? 1:0);
}