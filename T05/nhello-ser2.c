/**
* FILENAME : nhello-ser2.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#include "bor-util.h"



struct adr_compt{
	char adresse[108];
	int compteur;
};

int chercher(char *adresse,struct adr_compt *tab){
	int index = 0;
	while(strcmp(adresse,tab[index].adresse)  != 0 && index < 10000) index++;
	if(index < 10000) return index;
	return -1;
}

void incrementer_compteur_adresse(char *adresse,struct adr_compt *tab){
	int index = chercher(adresse,tab);
	if(index >= 0){
		tab[index].compteur++;
	}
}

int dialoguer_avec_un_client(int soc , struct adr_compt *tab_compteur){
	
	char buf1[2048], buf2[2048];
	
	struct sockaddr_un adr_client;
	
	static int compteur_client = 0;
	
	printf("Attente message client ...\n");
	
	int k = bor_recvfrom_un_str(soc , buf1 , sizeof(buf1), &adr_client);
	if(k<0) return -1;
	
	printf("Attente de recevoir  connection\n");
	
	if(strcmp(buf1,"CONNECTER") == 0){
			printf("client %s connecté\n",adr_client.sun_path);
			strcpy(tab_compteur[compteur_client].adresse, adr_client.sun_path);
			tab_compteur[compteur_client].compteur = 0;
			compteur_client++;
	}
	
	printf("Attente de HELLO\n");

	do{
		int k = bor_recvfrom_un_str(soc , buf1 , sizeof(buf1), &adr_client);
		if(k < 0) return -1;
		if(strcmp(buf1,"HELLO") == 0){
			printf("HELLO\n");
			int index = chercher(adr_client.sun_path,tab_compteur);
			if(index >= 0){
				tab_compteur[index].compteur++;

			}
			else printf("index = -1\n");
		}
		
	}while(strcmp(buf1,"NUMBER") != 0);
	
	printf("NUMBER recu \n");
	
	k = bor_recvfrom_un_str(soc , buf1 , sizeof(buf1), &adr_client);
	if(k < 0) return -1;
	
	printf("nb clients = %d\n ",compteur_client);
	
	//On fabrique la réponse
	int index  = chercher(adr_client.sun_path,tab_compteur);
	sprintf(buf2, "NUMBER : %d", tab_compteur[index].compteur);
	
	if(strcmp(buf1,"DECO") == 0){
		printf("client %s déconnecté\n",adr_client.sun_path);
		int index  = chercher(adr_client.sun_path, tab_compteur);
		if(index >= 0){
			strcpy(tab_compteur[compteur_client].adresse, tab_compteur[index].adresse);
			tab_compteur[index].compteur = tab_compteur[compteur_client].compteur;
			compteur_client--;
		}
	}
	
	printf("nb clients = %d\n ",compteur_client);
	
	printf("Envoi de la réponse...\n");
	
	k = bor_sendto_un_str(soc, buf2, &adr_client);
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
	bor_signal(SIGINT, Capter_SIGINT, 0);
	struct adr_compt tab_compteur[1000];
	char *nom_serveur = argv[1];
	struct sockaddr_un adr_serveur;
	
	printf("création de la socket \n");
	int soc = bor_create_socket_un(SOCK_DGRAM, nom_serveur, &adr_serveur);
	if(soc < 0) exit(1);
	printf("dans la boucle \n");
	while(boucle_princ){
		int k = dialoguer_avec_un_client(soc, tab_compteur);
		if(k < 0) break;
	}
	printf("Fermeture socket local\n");
	close(soc);
	unlink(adr_serveur.sun_path);
	exit(0);
}
