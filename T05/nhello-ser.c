/**
* FILENAME : nhello-ser.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#include "bor-util.h"

int dialoguer_avec_un_client(int soc){
	char buf1[2048], buf2[2048];
	struct sockaddr_un adr_client;
	int compt = 0;
	
	printf("Attente message client ...\n");
	do{
		int k = bor_recvfrom_un_str(soc , buf1 , sizeof(buf1), &adr_client);
		if(k<0) return -1;
		if(strcmp(buf1, "HELLO") == 0) compt++;
	}while(strcmp(buf1,"NUMBER") != 0);
	//On fabrique la réponse
	sprintf(buf2, "NUMBER : %d", compt);
	printf("Envoi de la réponse...\n");
	int k = bor_sendto_un_str(soc, buf2, &adr_client);
	if(k < 0) return -1;
	
	printf("Envoié %d octets à %s : \"%s\" \n", k, adr_client.sun_path, buf2);
	return 1;
}

int boucle_princ = 1;

void Capter_SIGINT(int sig){
	printf("Signal %d capté\n", sig);
	boucle_princ = 0;
}

int main (int argc, char *argv[]){
	if((argc - 1) != 1){
		fprintf(stderr, "Usage : %s socket_serveur\n", argv[0]);
		exit(1);
	}
	char *nom_serveur = argv[1];
	bor_signal(SIGINT, Capter_SIGINT, 0);
	struct sockaddr_un adr_serveur;
	int soc = bor_create_socket_un(SOCK_DGRAM, nom_serveur, &adr_serveur);
	if(soc < 0) exit(1);
	
	while(boucle_princ){
		int k = dialoguer_avec_un_client(soc);
		if(k < 0) break;
	}
	
	printf("Fermeture socket local\n");
	close(soc);
	unlink(adr_serveur.sun_path);
	exit(0);
}
