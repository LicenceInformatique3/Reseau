/**
* FILENAME : pair-cli.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include <bor_util.h>


int dialoguer_avec_serveur(int soc){
	char buf1[2000],buf2[2000];
	printf("construction message\n");
	int k=bor_read_str(0,buf2,sizeof(buf2));
	if( k < 0) return k;

	printf("envoi %d a %d \"%s\"\n",k,soc,buf2 );
	k=bor_write(soc,buf2);
	if( k < 0) return k;
	
	printf("attente réponse ...\n");
	int k=bor_read_str(soc,buf1,sizeof(buf1));
	if( k < 0) return k;
	
	printf("reçu %d de %d \"%s\"\n",k,soc,buf1 );
	return k;
}

int boucle_princ =1;

void capter_SIGINT(int sig){
	printf("Signal %d capté\n", sig);
	boucle_princ=0;
}

int main(int argc , char * argv[]){
	bor_signal(SIGINT,capter_SIGINT,0);
	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART);
	if(argc-1 != 2){
		fprintf(stderr, "Usage : %s adresse_socket_locale adresse_serveur", argv[0]);
		exit(1);
	}
	int k=-1;
	char * nom_client=argv[1];
	char * nom_serveur=argv[2];


	struct sockaddr_un adr client , adr_serveur;

	int soc = borcreate_socket_un(SOCK-STREAM,nom_client,adr_client);//un = domaine unix SOCk_STREAM=TCP
	if (soc < 0) exit(1);

	bor_set_sockaddr_un(nom_serveur,&adr_serveur);
	printf("connexion ...\n");
	if (bor_connect_un(soc,&adr_serveur)<0){
		goto fin1;
	}

	while(boucle_princ){
		k=dialoguer_avec_serveur(soc);
		if(k<=0) break;
	}

	fin1:
	printf("Fermeture client\n");
	close(soc);
	unlink(adr_client,sun_path);
	exit(k < 0 ? 1 : 0);
}