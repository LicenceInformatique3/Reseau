/**
* FILENAME : nhello-cli.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#include "bor-util.h"

int dialoguer_avec_serveur(int soc, struct sockaddr_un *adr_serveur){
	char buf1[2048], buf2[2048];
	
	printf("Envoi de la requète...\n");
	sprintf(buf2, "CONNECTER");
	printf("j'envoi connecter\n");
	int k = bor_sendto_un_str(soc, buf2, adr_serveur);
	if(k < 0) return -1;
	printf("connecter envoyer\n");

	sprintf(buf2, "HELLO");
	int n = 10000;
	for(int i = 0; i < n; i++){
		usleep(10);
		printf("J'écris %s\n", buf2);
		int k = bor_sendto_un_str(soc, buf2, adr_serveur);
		if(k < 0) {
			if(errno == ENOBUFS){
				continue;
			}
			else return -1;
		}
	}
	sprintf(buf2, "NUMBER");
	printf("j'envoi NUMBER\n");

	k = bor_sendto_un_str(soc, buf2, adr_serveur);
	if(k < 0) return -1;
	printf("NUMBER envoyer\n");
	sprintf(buf2, "DECO");
	printf("j'envoi DECO\n");
	
	k = bor_sendto_un_str(soc, buf2, adr_serveur);
	if(k < 0) return -1;
	printf("DECO envoyer\n");
	printf("Attente réponse serveur...\n");
	
	k = bor_recvfrom_un_str(soc , buf1 , sizeof(buf1), adr_serveur);
	if(k<0) return -1;
	
	printf("Reçu %d octets de %s : /” %s \"\n",k,adr_serveur->sun_path, buf1);
	return 1;
}

int dialoguer_avec_serveur_securise(int soc, struct sockaddr_un *adr_serveur){
	char buf1[2048], buf2[2048];
	struct sockaddr_un adr_tmp;
	sprintf(buf2, "HELLO");
	printf("Envoi de la requète...\n");
	
	int n = 10000;
	for(int i = 0; i < n; i++){
		int k = bor_sendto_un_str(soc, buf2, adr_serveur);
		if(k < 0) return -1;
	}
	
	sprintf(buf2, "NUMBER");
	int k = bor_sendto_un_str(soc, buf2, adr_serveur);
	if(k < 0) return -1;
	printf("Envoié %d octets à %s : /” %s \" \n", k, adr_serveur->sun_path, buf2);
	printf("Attente réponse serveur...\n");
	
	k = bor_recvfrom_un_str(soc , buf1 , sizeof(buf1), &adr_tmp);
	if(k<0) return -1;
	if(strcmp(adr_serveur->sun_path, adr_tmp.sun_path) != 0){
		printf("ATTENTION : ce n'est pas le bon serveur qui a répondu\n");
	}
	printf("Reçu %d octets de %s : /” %s \" \n",k, adr_tmp.sun_path, buf1);
	return 1;
}

int main (int argc, char *argv[]){
	if((argc - 1) != 2){
		fprintf(stderr, "Usage : %s adresse_client adresse_serveur\n", argv[0]);
		exit(1);
	}
	char *nom_client = argv[1],
	*nom_serveur = argv[2];
	struct sockaddr_un adr_client, adr_serveur;
	int soc = bor_create_socket_un(SOCK_DGRAM, nom_client, &adr_client);
	if(soc < 0) exit(1);
	//fabrique adr_serveur
	bor_set_sockaddr_un(nom_serveur, &adr_serveur);	
	int k = dialoguer_avec_serveur(soc, &adr_serveur);
	printf("Fermeture socket local\n");
	close(soc);
	unlink(adr_client.sun_path);
	exit( k < 0 ? 1 : 0);
}
