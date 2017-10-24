Tp: simuler un seveur daytime

while true ; do
	date | netcat -lv
	13000;
	done

simuler client
netcat localhost 13000

squelette client

socket
bind 					bor_create_socket_in

gethostbyname 			bor_resolve_address_in

connect
while 1
	read
	0	break
	affichage lu
close

dialoguer_avec_serveur
	bor_read_str
	printf

int main(int argc, char * argv[]){
	if(arfc-1 !=1){
		printf("usage : %s nomserveur\n", argv[0]);
		exit(1);
	}
	char * nomserveur=argv[1];
	struct sockaddr_in(adr_client,adr_serveur);
	int soc=bor_create_socket_in(sock_STREAM,0,&adr_client);
	if(soc <0){
		exit(1);
	}
	if(bor_resolve_addess_in(nom_serveur,13,&adr_serveur) <0){
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
		if(k<=0){
			 break;
		}
	}


	fin1 :
	printf("Fin client\n");
	close(soc);
}

modif 
~if(arfc-1 !=2){
		printf("usage : %s nomserveur\n", argv[0]);
		exit(1);
	}
+ port_serveur=atoi(argv[2]);

~ if(bor_resolve_addess_in(nom_serveur,port_serveur,&adr_serveur) <0){

+ char buf[1000];
int pos=0;
while (1){
	k=bor_read_str(soc,buf+pos,sizeof(buz)-pos);
	if(k<=0){
		break;
	}
	printf("client : a lu ..");
	defragmenter_ligne(buf,sizeof(buf),&pos,&pos +k); //pos +=k
}

int couper_ligne(char * buf,int debut,int fin){
	for(int i=debut,i< fin,i++){
		if(buf[i]=='\n'){
			buf[i]='\0';
			return i;
		}
	}
	return -1;
}

void supprimer ligne(char * buf , int i , int * fin){
	memmove(buf,buf+i *fin-i);
	*fin -= i+1;
}

void defragmenter_ligne(char *buf ,int bufsize,int * pos,int fin){
	int i, debut=*pos;
	while((i=couper_ligne(buf,debut,fin))>=0){
		printf("1 ligne détectée : \"%s\"\n",buf);
		supprimer_ligne(buf,i,&fin);
		debut=0;
	}
	printf("Reste dans buf ; \"%s\"\n",buf );
	*pos=fin;
	if(*pos >= bufsize -1){
		printf("buffer plein\n");
		*pos=0;
		buf[0]=0;
	}
}