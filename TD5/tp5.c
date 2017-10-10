int dialoguer_avec_serveur

int main (int argc,char * argv[]){
	if(argc-1 !=2){
		fprintf(stderr, "usage :  %s Client nom_clé nom ser \n",argv[0] );
		exit(1);
	}
	char * nom_client = argv[1];
	char * nom_serveur = argv[2];
	struct sockaddr_un adr client,adr_serveur;
	//bor_signal(SIGINT,capter_SIGITN,0);
	
	int soc = bor_create_socket_un(SOCK_DGRAM,nom_client,&adr_client);
	if(soc <0){
		exit(1);
	}
	//fabrique adr serveur
	bor_set_sockaddr_un(nom_serveur,&adr_serveur);
	int k=dialoguer_avec_serveur(soc,&adr_serveur);
	printf("Fermeture socket locale\n");
	close(soc);
	unlink(adr_client.sun_path);
	exit(k<0?1:0);
}
/*
	while(boucle_prime){
		int k=dialoguer_avec_un_client(soc);
		if(k<0){
			break;
		}
	}
	printf("Fermeture socket locale\n");
	close(soc);
	unlink(adr_serveur.sun_path);
	exit(0);
}*/