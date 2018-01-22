/**
* FILENAME : serveur2-tcpip.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include "bor-util.h"

typedef enum { E_LIBRE, E_LIRE_REQUETE, E_ECRIRE_REPONSE} Etat;

typedef struct {
	Etat etat;
	int soc;
	struct sockaddr_in adr;
}Slot;


#define SLOTS_NB 32

typedef struct{
	Slot slots[SLOTS_NB];
	int soc_ec;			 //Socket d'écoute
	struct sockaddr_in adr; //Adresse du serveur
} Serveur;

void init_slot(Slot *o){
	o->etat= E_LIBRE;
	o->soc =-1;
	memset(&0->adr,0,sizeof o->adr);	//non obligatoire
}

int slot_est_libre(Slot *o){
	return o->etat==E_LIBRE;
}

void liberer_slot(Slot *o){
	if(slot_est_libre(o)) return;
	close(o->soc);
	init_slot(o);
}

void init_serveur(Serveur * ser){
	for (int i = 0; i < SLOTS_NB; ++i)
		init_slot(&ser->slots[i]);
	ser->soc_ec=-1;
	memset(&ser->adr,0,sizeof (ser->adr));  //non obligatoire
}

int chercher_slot_libre(Serveur *ser){
	for (int i = 0; i < SLOTS_NB; ++i)
		if (slot_est_libre(&ser->slots[i]))
			return i;
	return -1;
}

int demarrer_serveur(Serveur *ser,int port){
	init_serveur(ser);
	ser->soc_ec=bor_create_socket_in(SDCK_STREAM,port,&ser->adr);
	if(ser->soc_ec<0) return -1;
	if(bor_listen(ser->soc_ec,8)<0){
		close(ser->soc_ec);
		return -1;
	}
	return 0;
}

void fermer_serveur(Serveur *ser){
	close(ser->soc_ec);
	for (int i = 0; i < SLOTS_NB; ++i)
		liberer_slot(&ser->slots[i]);
}

int accepter_connexion(Serveur *ser){
	struct sockaddr_in adr_client;
	printf("connexion en cours ...\n");
	int soc_se=bor_accept_in(ser->soc_ec,&adr_client);
	if (soc_se<0) return -1;
	int i=chercher_slot_libre(ser);
	if(i<0){
		close(soc_se);
		printf("Serveur : connexion refusée avec %s : plus de Slot libre\n",bor_adrtoa_in(&adr_client) );
		return 0;
	}
	printf("Serveur %d connexion établie avec %s\n",soc_se,bor_adrtoa_in(&adr_client) );
	Slot *o = &ser->slots[i];
	o->soc=soc_se;
	o->adr=adr_client;
	o->etat=E_LIRE_REQUETE;
	return 1;
}

int proceder_lecture_requete(Slot *o){
	char buf[1024];
	int k=bor_read_str(o->soc,buf,sizeof(buf));
	if(k<0) return k;
	o->etat=E_ECRIRE_REPONSE;
	printf("reçu %d de %d \"%s\"\n",k,o->soc,buf );
	return k;
}

int proceder_ecriture_reponse(Slot *o){
	char buf[1024];
	sprintf(buf, "HTTP/1.1 500 Erreur du \r\n\r\n <html><body><h1>Serveur en construction !! </h1></body></html>\r\n");
	int k=bor_write_str(o->soc,buf);
	if(k<0) return k;
	o->etat=E_LIRE_REQUETE;
	printf("envoyé %d à %d \"%s\"\n",k,o->soc,buf);
	return k;
}

void traiter_slot_si_eligible(Slot *o, fd_set *set_read, fd_set *set_write){
	int k=1;
	if(slot_est_libre(o)) return;
	switch (o->etat){
		case E_LIRE_REQUETE:
			if(FD_ISSET(o->soc,set_read))
				k=proceder_lecture_requete(o);
			break;
			
		case E_ECRIRE_REPONSE:
			if(FD_ISSET(o->soc,set_write))
				k=proceder_ecriture_reponse(o);
			break;
		default:;
	}
	if(k<=0){
		printf("Serveur %d : libération Slot \n",o->soc );
		liberer_slot(o);
	}
}

void inserer_fd(int fd, fd_set *set, int *maxfd){
	FD_SET(fd,set);
	if(*maxfd <fd)
		maxfd=fd;
}

void preparer_select( Serveur *ser, fd_set *set_read, fd_set *set_write, int *maxfd){
	FD_ZERO(set_read);
	FD_ZERO(set_write);
	*maxfd=-1
	inserer_fd(ser->soc_ec,set_read,maxfd);
	for (int i = 0; i < SLOTS_NB; ++i){
		Slot *o=&ser->slots[i];
		if(slot_est_libre(o))
			continue;
		switch(o->etat){
			case E_LIRE_REQUETE :
				inserer_fd(o->soc,set_read,maxfd);
				break;
			case E_ECRIRE_REPONSE :
				inserer_fd(o->soc,set_write,maxfd);
				break;
			default:;
		}
	}
}

int faire_scrutation(Serveur * ser){
	int maxfd;
	fd_set set_read, set_write;
	preparer_select(ser,&set_read,&set_write,&maxfd);
	int res = select(maxfd+1, &set_read, &set_write, NULL, NULL);
	if (res < 0){
		if(errno == EINTR)
			printf("Un signal a été capté.\n");
		else {
			perror("select");
			return -1;
		}
	}
	if (FD_ISSET (ser->soc_ec, &set-read)){
		if(accepter_connexion(ser)<0){
			return -1;
		}
	}
	for (int i = 0 ; i < SLOTS_NB ; i++){
		Slot* o = &ser->slots[i];
		traiter_slot_si_eligible(o,&set_read,&set_write);
	}
	return 1;
}

int boucle_princ =1;

void capter_SIGINT(int sig){
	boucle_princ=0;
	printf("Signal %d capté : Arret serveur\n", sig);
}

int main(int argc , char * argv[]){
	if(argc-1 != 1){
		fprintf(stderr, "usage : %s numeroPort\n", argv[0]);
		exit(1);
	}
	bor_signal(SIGINT,capter_SIGINT,0);
	int port=atoi(argv[1]);
	Serveur ser;
	int k=demarrer_serveur(&ser,port);
	if (k!=0)
		goto fin1;

	while(boucle_princ){
		k=faire_scrutation(&ser);		   //inti_select_selon_etat	select  test éligibilité	soc_ec->accepter_connexion  Slot 
		if( k <= 0 ) break;
	}

	fin1;
	fermer_serveur(&ser);
	return 0;
}
