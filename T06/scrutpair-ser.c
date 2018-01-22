/**
* FILENAME : scruptpair-ser.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include "bor-util.h"

void preparer_reponse_non_vide(char *buf1, char * buf2){
	int j=0;
	for(int i=0; buf1[i];i++){
		if (buf1[i]>='0' && buf1[i] <= '9' && (buf1[i]-'0' %2 ==0)){
			buf2[j++]=buf1[i];
		}
	}
	if(j==0){
		buf2=' ';
	}
	buf2[j]='\0';
}

int dialoguer_avec_client(int soc){
	char buf1[2000],buf2[2000];
	printf("Fils attends demande ...\n");
	int k=bor_read_str(soc,buf1,sizeof(buf1));
	if(k < 0) return k;

	printf("reçu %d de %d \"%s\"\n",k,soc,buf1 );
	preparer_reponse_non_vide(buf1,buf2);
	printf("envoi a %d \"%s\"\n",soc,buf2 );
	k=bor_write(soc,buf2);
	return k;
}

void deconnexion(int index, int *tab_socket_service, int *compteur){
	printf("Déconnexion \n");
	close(tab_socket_service[index]);
	tab_socket_service[index] = tab_socket_service[*compteur-1];
	*compteur = *compteur -1;
}

int Max_descripteur(int *tab_socket_service, int compteur){
	int max = -1;
	for (int i = 0; i < compteur; i++) {
		if(max < tab_socket_service[i]) max = tab_socket_service[i];
	}
	return max;
}

int boucle_princ = 1;

void capter_SIGINT(int sig){
	printf("Signal %d capté\n", sig);
	boucle_princ = 0;
}

int Connexion_Demande_clients(int socec, int *tab_socket_service, int *compteur){
	
	struct sockaddr_un adr_client;
	fd_set set1;
	int r;
	FD_ZERO(&set1);
	FD_SET(socec, &set1);
	
	for(int i = 0; i < *compteur ; i++){
		FD_SET(tab_socket_service[i], &set1);
	}
		
	int max = Max_descripteur(tab_socket_service, *compteur);
	if(max < socec) max = socec;
	if(max > 0){
			
	r = select((max)+1, &set1, NULL, NULL, NULL);
	if(r < 0){
		if(errno == EINTR) printf("Un signl a été capté.\n");
		else perror("select");
	}
		
	if(r == 0) printf("Délai écoulé\n");
		
	if(FD_ISSET(socec, &set1)){
			printf("Attente de connexion...\n");
			int socse = bor_accept_un(socec, &adr_client);
			tab_socket_service[*compteur] = socse;
			*compteur = *compteur + 1;
			printf("Connexion établie avec %s\n", adr_client.sun_path);
	}
	for(int i = 0 ; i < *compteur; i++ ){
		if(FD_ISSET(tab_socket_service[i], &set1)){	
			int k = dialoguer_avec_client(tab_socket_service[i]);
			if(k <= 0 || boucle_princ == 0){
				deconnexion(i, tab_socket_service, compteur);
				i--;
				}
			}
		}
	}
	return 1;
}



int main (int argc, char *argv[]){
	bor_signal(SIGINT,capter_SIGINT,0);
	bor_signal(SIGCHLD,capter_SIGCHLD,SA_RESTART);
	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART);
	if(argc-1 != 1){
		fprintf(stderr, "Usage : %s adresse_serveur\n", argv[0]);
		exit(1);
	}
	int k=-1;
	char * nom_serveur=argv[1];

	struct sockaddr_un adr_serveur;
						
	int compteur = 0;
	int tab_socket_service[100];
						
	int socec = borcreate_socket_un(SOCK-STREAM,nom_serveur,adr_serveur);//un = domaine unix SOCk_STREAM=TCP
	if(socec < 0) exit(1);
	if(bor_listen(socec, 1) < 0) goto fin1;
	//peut échouer si socket n'est pas SOCK_STREAM
						
	while(boucle_princ){
		k = Connexion_Demande_clients(socec, tab_socket_service, &compteur);
		if(k <= 0) break;
	}					
fin1:
	printf("Fermeture Serveur\n");
	close(socec);
	unlink(adr_serveur.sun_path);					 
	exit(k < 0 ? 1 : 0);	 
}
