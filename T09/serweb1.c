typedef struct slot
{
	Etat etat;
	int soc;
	struct sockaddr_in adr;
	char req[REQSIZE]
	int req_pos, fin_entete;
}slot;

void init_slot(slot *o){
	o->etat=E_LIBRE;
	o->soc=-1;

	o->req[0] = '\0';
	o->req_pos=0;
	o->fin_entete=0;
}

int lire_suite_requete(slot *o){
	int k=bor_read_str(
		o->soc,
		o->req+o->req_pos,
		REQSIZE-o->req_pos);
	if(k>0) 
		o->req_pos+=k;
	return k;
}

typedef struct slot
{
	Etat etat;
	int soc;
	struct sockaddr_in adr;
	char rep[REPSIZE]
	int rep_pos, 
}slot;

void init_slot(slot *o){
	o->etat=E_LIBRE;
	o->soc=-1;

	o->rep[0] = '\0';
	o->rep_pos=0;
}

int ecrire_suite_reponse(slot *o){
	int k=bor_write_str(
		o->soc
		o->rep+o->rep_pos);
	if(k>0) 
		o->rep_pos+=k;
	return k;
}

int chercher_fin_entete(slot *o, int debut){
	char *s=strstr(o->req+debut,"\n\n");
	if(!s)
		return s-o->req;
	s=strstr(o->req+debut,"\n\n");
	return s ? s-o->req : -1;
}

int chercher_fin_requete(slot *o, int debut){	//chercher_fin_entete
	for (int i = debut; o->req[i]; ++i){
		if ((o->req[i]=='\n' && o->req[i+1]=='\n' ) || 
			(o->req[i]=='\r' && o->req[i+1]=='\n' && o->req[i+2]=='\r' && o->req[i+3]=='\n')){
			return i;
		}
	}
	return -1;
}

char * get_http_error_message(code_reponse code){
	switch(code){
		case C_OK : return "Ok";
		case C_BAD_REQUEST : return "Bad request";
		//case :
		default : return "Other error";
	}
}

Id_method get_id_method(chat * methode){
	if(!strcasacmp(methode,"GET"))
		return M_GET;
	if(!strcasacmp(methode,"TRACE"))
		return M_TRACE;
	return M_NONE;
}

typedef struct Infos_entete{
	char methode[REQSIZE],
		url[REQSIZE],
		version[REQSIZE],
		chemin[REQSIZE];

	Id_method id_meth;
	code_reponse reponse;
}Infos_entete;

void analyser_requete(slot *o, Infos_entete *ie){
	//provisoire
	(void)o;
	ie->reponse=C_NOT_FOUNT;
}

void preparer_reponse(slot *o, Infos_entete *ie){
	//provisoire
	(void)ie;
	sprintf(o->rep,REPSIZE,"HTTP/1.1 404 Notfound \r\n"
							"Serveur: servweb1 \r\n"
							"Connection: close \r\n"
							"Content-Type: text/html \r\n\r\n"
							"<html><head><title></title></head>\r\n"
							"<body><h1></h1></body></html>\r\n");
}

int proceder_lecture_requete(slot *o){
	int prec_pos=o->req_pos;
	int k=lire_suite_requete(o);
	if(k<=0)
		return -1;
	int debut=prec_pos_-3;
	if(debut<0)
		debut=0;
	o->fin_entete=chercher_fin_entete(o,debut);
	if(o->fin_entete<0){
		printf("Serveur[%d]: requète incomplète\n",o->soc );
		return 1;
	}
	//requete complete
	printf("Serveur[%d]: reçu requète \"%s\"\n",o->soc,o->rep);
	Infos_entete ie;
	analyser_requete(o,&ie);
	preparer_reponse(o,&ie);
	o->etat=E_ECRIRE_REPONSE;
	return 1;
}

int proceder_ecriture_reponse(slot *o){
	int k=ecrire_suite_reponse(o);
	if(k<0)
		return -1;
	if(o->rep_pos < (int) strlen(o->rep)){
		printf("Serveur[%d]: réponse incomplète\n",o->soc );
		return 1;
	}
	//ici on changera d'état pour envoyer le fichier demandé
	//provisoire :on déconnecte
	return 0;

}