int boucle_princ =1;

void capter_SIGINT(int sig){
	boucle_princ=0;
}

int main(){
	bor_signal(SIGINT,capter_SIGINT,0);
	bor_signal(SIGPIPE,SIG_IGN,SA_RESTART);
	if(argc-1 != 2){

	}
	int k=-1;
	char * nom_client=argv[1];
	char * nom_serveur=argv[2];


	struct sockaddr_un adr client , adr_serveur;

	int soc = borcreate_socket_un(SOCK-STREAM,nom_client,adr_client);//un = domaine unix SOCk_STREAM=TCP
	if (soc < 0){
		exit(1);
	}
	bor_set_sockaddr_un(nom_serveur,&adr_serveur);
	printf("connexion ...\n");
	if (bor_connect_un(soc,&adr_serveur)<0){
		goto fin1;
	}

	while(boucle_princ){
		k=dialoguer_avec_serveur(soc)
		if(k<=0){
			break;
		}
	}

	fin1;
	printf("Fermeture client\n");
	close(soc);
	unlink(adr_client,sun_path);
	exit(k < 0 ? 1 : 0);
}