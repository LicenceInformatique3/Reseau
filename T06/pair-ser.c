/**
* FILENAME : pair-ser.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include <bor_util.h>


void capter_SIGCHLD(int sig){
	printf("signal %d capter \n", sig);
	int j;
	while((j=waitpid(-1,NULL,WNOHANG))>0){
		printf("Elimination du zombie %d \n",j );
	}
}

int boucle_princ =1;

void capter_SIGINT(int sig){
	printf("Signal %d capté\n", sig);
	boucle_princ=0;
}

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

void fils_main(int socec,int socse,struct sockaddr_un adr_client){
	int k=-1;
	close(socec);

	while(boucle_princ){
		k=dialoguer_avec_client(socse);
		if(k<=0) break;
	}

	printf("Fermeture fils\n");
	close(socse);
	exit(k < 0 ? 1 : 0);
}

int traiter_connexion_client(int socec){
	struct sockaddr_un adr_client;
	printf("Attente connexion ...\n");
	int socse=bor_accept_un(socec,&adr_client);
	if (socse < 0) return -1;
	printf("Connexion établie avec %s\n", adr_client.sun_path);	//adr_client
	int p=fork();
	if (p<0)	{
		perror("fork");
		return -1;
	}
	if (p==0){
		fils_main(socec,socse,&adr_client);
		exit(0);
	}
	printf("pere disponible\n");
	close (socse);
	return 1;
}

int main(int argc , char * argv[]){
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

	int socec = borcreate_socket_un(SOCK-STREAM,nom_serveur,adr_serveur);//un = domaine unix SOCk_STREAM=TCP
	if(soc_ec < 0) exit(1);
	if(bor_listen(soc_ec, 1) < 0) goto fin1;

	while(boucle_princ){
		k=traiter_connexion_client(socec);
		if(k <= 0) break;
	}

	fin1:
	printf("Fermeture serveur\n");
	close(soc);
	unlink(adr_serveur,sun_path);
	exit(k < 0 ? 1 : 0);
}