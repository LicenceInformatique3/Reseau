/**
* FILENAME : daytime.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include "bor-util.h"

int dialoguer_avec_serveur(int soc){
	char buf[1024];
	int k=bor_read_str(soc,buf,sizeof(buf));
	if(k<=0) return k;
	printf("reçu %d de %d \"%s\"\n",k,soc,buf );
	return k;
}

int main(int argc, char * argv[]){
	if(argc-1 !=1){
		printf("usage : %s nomserveur\n", argv[0]);
		exit(1);
	}
	char * nom_serveur=argv[1];
	int port = 13000;
	int k = -1;

	struct sockaddr_in adr_serveur;
	int soc=bor_create_socket_in(SOCK_STREAM,0,&adr_serveur);
	if(soc <0) exit(1);
	if(bor_resolve_address_in(nom_serveur,port,&adr_serveur) <0){
		goto fin1;
	}
	printf("connexion ...");

	if(bor_connect_in(soc,&adr_serveur)<0){
		goto fin1;
	}
	printf("connexion établie");
	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART); // inutile car que read ici mais necessaire en réseau dès utilisation de write

	while(1){
		k=dialoguer_avec_serveur(soc);
		if(k<=0) break;
	}

	fin1 :
	printf("Fin client\n");
	close(soc);
	exit(k<0?1:0);
}