#include "bor-util.h"

typedef struct {
	char *fnofam_nom,
	* tub_ec_nom,
	tub_cs_nom[100],
	tub_sc_nom[100];
	int tub_ec,tub_sc,tub_cs;
}Serveur;


int boucle_princ=1;
void capter_fin(int sig){
	boucle_princ=0;
}




int creer_tubes_ecoute(Serveur *s){
	printf("Création tubes ecoute\n");
	sprintf(s->tub_ec_nom,"tub_ec_%d",(int)getpid());
	int k=mkfifo(s->tub_ec_nom, 0600);
	if (k<0){
		perror("mkfifo cs");
		return -1;
	}
	return 0
}


void supprimer_tubes_ecoute(Serveur *s){
	printf("Suppresion tubes ecoute\n");
	unlink(s->tub_ec_nom);
}

int ouvrir_tubes_ecoute(Serveur *s){
	//ouvertures bloquantes !!
	printf("ouvertures tub_ec ecoute\n");
	s->tub_ec=open(s->tub_ec_nom,O_RDONLY);
	if (s->tub_ec<0){
		perror("open tub_cs");
		return -1;
	}
	return 0;
}

void fermer_tubes_ecoute(Serveur *s){
	printf("Fermetures tub_ec ecoute\n");
	close(c->tub_ec);

}


void main_fils();
			/*filsmain
				ouvrir tube service
				while (boucle princ)
					faire dialogue
				fermer tube service*/

int attendre_contact(Serveur *s){
	int k=ouvrir_tubes_ecoute(s);
	if (k<0){
		return k;
		//lecture dans tub-ec des  nom tube service + attribution
	}
}

int faire_transaction(Serveur *s){
	attendre_contact(s);
	int f=fork();
	if (f<0){
		perror("fork")
		return -1;
	}
	if (f==0){
	fermer_tubes_ecoute(s);
	main_fils(s);
	}
	while(read > 0){
	}
	fermer_tubes_ecoute(s);
	return 0;
}
/*
	faire_transaction
		attendre_contact

			faire_diaogue
				lecture tub_cs
				recherche nam
				écriture tub-sc
*/

/*
int prendre_contact(Client *c){
	printf("Ouverture tub_ec \n");
	if (open(c->tub_ec_nom,O_WRONLY) <0){
		perror ("open tub_ec")
		return -1;
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

}*/

int main(int argc,char * argv[]){
	Serveur s;
	int r=-1;
	if (argc-1 != 1){
		fprintf(stderr, "Usage: %s tub_ec\n", argv[0] );
		exit(1);
	}
	
	if(creer_tubes_ecoute(&s) < 0){
		exit(1);
	}
	bor_signal(SIGINT,capter_fin,0);
	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART);	
	bor_signal(SIGCHLD,SIG_IGN,SA_RESTART);	

	while(boucle_princ){

	}

	supprimer_tubes_ecoute(&s);
	exit 0;
}