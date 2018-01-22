/**
* FILENAME : defrag.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include "bor-util.h"

int couper_ligne(char * buf,int debut,int fin){
	for(int i=debut;i< fin;i++){
		if(buf[i]=='\n'){
			buf[i]='\0';
			return i;
		}
	}
	return -1;
}

void supprimer_ligne(char * buf , int i , int * fin){
	memmove(buf, buf+i+1, *fin-i);
	*fin -= i+1;
}

void defragmenter_ligne(char *buf ,int bufsize,int * pos,int fin){
	int i, debut=*pos;

	while((i=couper_ligne(buf,debut,fin))>=0){
		printf("1 ligne détectée : \"%s\"\n",buf);
		supprimer_ligne(buf,i,&fin);
		debut=0;
	}

	printf("Reste dans buf : \"%s\"\n",buf );
	*pos=fin;

	if(*pos >= bufsize -1){
		printf("buffer plein\n");
		*pos=0;
		buf[0]=0;
	}
}

int main(int argc, char * argv[]){
	if(argc-1 !=2){
		printf("usage : %s nomserveur port_serveur\n", argv[0]);
		exit(1);
	}
	char * nom_serveur=argv[1];
	int port_serveur=atoi(argv[2]);
	int k = -1;
	struct sockaddr_in adr_serveur;
	int soc=bor_create_socket_in(SOCK_STREAM,0,&adr_serveur);
	if(soc <0) exit(1);
	if(bor_resolve_address_in(nom_serveur,port_serveur,&adr_serveur) <0){
		goto fin1;
	}
	printf("connexion ...");
	if(bor_connect_in(soc,&adr_serveur)<0){
		goto fin1;
	}
	printf("connexion établie");
	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART); // inutile car que read ici mais necessaire en réseau dès utilisation de write
	char buf[1024];
	int pos=0;
	while (1){
		k=bor_read_str(soc,buf+pos,sizeof(buf)-pos);
		if(k<=0) break;
		printf("Client: a lu %s \n", buf);
		defragmenter_ligne(buf,sizeof(buf),&pos,pos +k); //pos +=k
	}

	fin1 :
	printf("Fin client\n");
	close(soc);

	exit(k<0?1:0);
}
