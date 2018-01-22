#include "bor-util.h"

int dialoguer_avec_serveur(int soc,char * chemin,FILE * f){
	char buf[1024];
	sprintf(buf,"GET /~thiel/essai.txt HTTP/1.0\r\n\r\n");
	//sprintf(buf,"GET /%s HTTP/1.0\r\n\r\n",chemin);
	printf("envoi a %d \"%s\"\n",soc,buf );
	k=bor_write(soc,buf2);
	if(k<=0) return k;
	printf("attente réponse ...\n");
	int k=bor_read_str(soc,buf,sizeof(buf));
	if(k<=0) return k;
	printf("reçu %d de %d \"%s\"\n",k,soc,buf );
	return k;
}

int main(int argc, char * argv[]){
	if(argc-1 !=4){
		printf("usage : %s addresse_machine num_port chemin nom_fichier\n", argv[0]);
		exit(1);
	}
	char * addresse_machine=argv[1];
	char * chemin=argv[3];
	char * nom_fichier=argv[3];
	int port=atoi(argv[2]);
	int k = -1;

	struct sockaddr_in adr_serveur;
	int soc=bor_create_socket_in(SOCK_STREAM,0,&adr_serveur);
	if(soc <0){
		exit(1);
	}
	if(bor_resolve_address_in(addresse_machine,port,&adr_serveur) <0){
		goto fin1;
	}
	printf("connexion ...");

	if(bor_connect_in(soc,&adr_serveur)<0){
		goto fin1;
	}
	printf("connexion établie");

	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART); // inutile car que read ici mais necessaire en réseau dès utilisation de write

	File *f = fopen(nom_fichier,"w");
	if (f==NULL){
		goto fin1;
	}

	while(1){
		k=dialoguer_avec_serveur(soc,chemin,f);
		if(k<=0) break;
	}

	fin1 :
	printf("Fin client\n");
	close(soc);
	exit(k<0?1:0);
}