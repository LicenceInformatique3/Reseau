/**
* FILENAME : date-cli.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#include "bor-util.h"
#include "bor-timer.h"


int dialoguer_avec_serveur(int soc,struct sockaddr_un *adr_serveur){
	struct sockaddr_un adr_client;
	char buf1[2048],buf2[2048];
	//on fabrique la requete
	sprintf(buf2,"HELLO");
	printf("Envoi de la requete ...\n");
	int k=bor_sendto_un_str(soc,buf2,adr_serveur);
	if(k<0) return -1;

	printf("Envoyé %d octets à %s : \"%s\"\n",k, adr_serveur->sun_path, buf2 );
	printf("Attente réponse serveur ...\n");
	k=bor_recvfrom_un_str(soc,buf1,sizeof(buf1),adr_serveur);
	printf("Reçu %d octets de %s : \"%s\"\n",k, adr_serveur->sun_path, buf1 );	
	return 1;
}

/*
int dialoguer_avec_serveur(int soc,struct sockaddr_un *adr_serveur){
	struct sockaddr_un adr_client;
	char buf1[2048],buf2[2048];
	//on fabrique la requete
	sprintf(buf2,"HELLO");
	printf("Envoi de la requete %s 10000 fois...\n",buf2);
	for (int i = 0; i < 10000; ++i){
		int k=bor_sendto_un_str(soc,buf2,adr_serveur);
		if(k<0){
			return -1;
		}
	}
	sprintf(buf2,"NUMBER");
	int k=bor_sendto_un_str(soc,buf2,adr_serveur);
	if(k<0){
		return -1;
	}
	printf("Attente réponse serveur ...\n");
	k=bor_recvfrom_un_str(soc,buf1,sizeof(buf1),adr_serveur);
	printf("Reçu %d octets de %s : \"%s\"\n",k,adr_serveur->sun_path,buf1 );	
	return 1;
}
*/

int dialoguer_avec_serveur_securise(int soc, struct sockaddr_un *adr_serveur){
	struct sockaddr_un adr_tmp;
	char buf1[2048], buf2[2048];
	//on fabrique la requete
	sprintf(buf2,"HELLO");
	printf("Envoi de la requete ...\n");
	int k = bor_sendto_un_str(soc, buf2, adr_serveur);
	if(k < 0) return -1;
	
	printf("Envoyé %d octets à %s : \"%s\"\n",k,adr_serveur->sun_path,buf2 );
	printf("Attente réponse serveur ...\n");
	k = bor_recvfrom_un_str(soc , buf1 , sizeof(buf1), &adr_tmp);
	if(k<0) return -1;
	
	if(strcmp(adr_serveur->sun_path, adr_tmp.sun_path) != 0){
		printf("ATTENTION : ce n'est pas le bon serveur qui a répondu\n");
	}
	printf("Reçu %d octets de %s : /” %s \" \n",k, adr_tmp.sun_path, buf1);
	return 1;
}

int boucle_prime=1;

void capter_SIGITN(int sig){
	printf("signal %d capté\n", sig);
	boucle_prime=0;
}

int main (int argc,char * argv[]){
	if(argc-1 !=2){
		fprintf(stderr, "usage :  %s Client nom_clé nom ser \n",argv[0] );
		exit(1);
	}
	char * nom_client = argv[1];
	char * nom_serveur = argv[2];
	struct sockaddr_un adr_client,adr_serveur;
	bor_signal(SIGINT,capter_SIGITN,0);
	
	int soc = bor_create_socket_un(SOCK_DGRAM,nom_client,&adr_client);
	if(soc <0) exit(1);
	//fabrique adr serveur
	int k;
	bor_set_sockaddr_un(nom_serveur,&adr_serveur);
	while(boucle_prime){
		k=dialoguer_avec_serveur(soc,&adr_serveur);
		if(k<0) break;
	}

	printf("Fermeture socket locale\n");
	close(soc);
	unlink(adr_client.sun_path);
	exit(k<0?1:0);
}