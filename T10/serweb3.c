/**
* FILENAME : serweb3.c
* AUTHOR : Moragues Lucas, Perrot Gaëtan
*
**/

#define _GNU_SOURCE
#include "bor-util.h"
#include "bor-timer.h"

#define REQSIZE 2048
#define REPSIZE 2048
#define FICSIZE 2048

typedef enum { E_LIBRE, E_LIRE_REQUETE, E_ECRIRE_REPONSE, E_LIRE_FICHIER, E_ENVOYER_FICHIER } Etat;

typedef struct {
	Etat etat;
	int soc;
	struct sockaddr_in adr;
	char req [REQSIZE];
	int req_pos, fin_entete;
	char rep [REPSIZE];
	int rep_pos;
	int fic_fd;
	int handle;
	char fic_bin[FICSIZE];
	int fic_pos;
	int fic_len;
} Slot;

#define SLOTS_NB 32
typedef struct
{
	Slot slots[SLOTS_NB];
	int soc_ec;			 //Socket d'écoute
	struct sockaddr_in adr; //Adresse du serveur
} Serveur;

typedef enum { M_NONE,M_GET,M_TRACE} Id_methode;

typedef enum {
	C_OK = 200,
	C_BAD_REQUEST = 400,
	C_NOT_FOUND = 404,
	C_METHOD_UNKNOWN = 501,
} Code_reponse;

typedef struct {
	char 	methode [REQSIZE],
			url [REQSIZE],
			version [REQSIZE],
			chemin [REQSIZE];
	Id_methode id_meth;
	Code_reponse code_rep;
	char type_mime[100];
	off_t file_size;
} Infos_entete;

void init_slot (Slot *o){
	o->etat = E_LIBRE;
	o->soc = -1;
	memset (&o->adr, 0, sizeof(o->adr)); // Non obligatoire
	o->req [0] = '\0';
	o->req_pos = 0;
	o->fin_entete = 0;
	
	o->rep [0] = '\0';
	o->rep_pos = 0;
	o->fic_fd = -1;
	o->handle = -1;
	
	o->fic_bin[0] = '\0';
	o->fic_pos = 0;
	o->fic_len = 0;
}

int slot_est_libre (Slot *o){
	return ( o->etat == E_LIBRE );
}

void liberer_slot (Slot *o){
	if ( slot_est_libre(o) ) return;
	close (o->soc);
	if (o->fic_fd != -1)
		close (o->fic_fd);
	if (o->handle != -1)
		bor_timer_remove(o->handle);
	init_slot (o);
}

void init_serveur (Serveur *ser){
	for (int i = 0; i < SLOTS_NB; i++)
		init_slot(&ser->slots[i]);
	ser->soc_ec = -1;
	memset (&ser->adr, 0, sizeof(ser->adr)); // Non obligatoire
}

int chercher_slot_libre (Serveur *ser){
	for (int i = 0; i < SLOTS_NB; i++)
		if (slot_est_libre(&ser->slots[i]))
			return i;
	return -1;
}

int demarrer_serveur (Serveur *ser, int port){
	init_serveur (ser);
	ser->soc_ec = bor_create_socket_in (SOCK_STREAM, port, &ser->adr);
	if (ser->soc_ec < 0) return -1;
	if (bor_listen (ser->soc_ec, 8) < 0){
		close (ser->soc_ec);
		return -1;
	}
	return 0;
}

void fermer_serveur (Serveur *ser){
	close (ser->soc_ec);
	for (int i = 0; i < SLOTS_NB; i++)
		liberer_slot(&ser->slots[i]);
}

int accepter_connexion (Serveur *ser){
	struct sockaddr_in adr_client;
	printf ("Connexion en cours...\n");
	int soc_se = bor_accept_in (ser->soc_ec, &adr_client);
	if (soc_se < 0) return -1;
	int i = chercher_slot_libre (ser);
	if (i < 0){
		printf ("Serveur : connexion refusée avec %s : plus de slot libre\n",bor_adrtoa_in (&adr_client) );
			return 0;
	}
	printf ("Serveur [%d] : connexion établie avec %s\n", soc_se, bor_adrtoa_in (&adr_client));
	Slot *o = &ser->slots[i];
	o->soc=soc_se;
	o->adr=adr_client;
	o->etat=E_LIRE_REQUETE;
	return 1;
}

int lire_suite_requete (Slot *o){
	int k = bor_read_str (o->soc, o->req + o->req_pos,REQSIZE - o->req_pos);
	if (k > 0)
		o->req_pos += k;
	return k;
}

int ecrire_suite_reponse (Slot *o){
	int k = bor_write_str (o->soc, o->rep + o->rep_pos);
	if (k > 0)
		o->rep_pos += k;
	return k;
}

int chercher_fin_requete_strstr (Slot *o, int debut){
	char *s = strstr (o->req + debut, "\n\n");
	if (!s) return (s - o->req);
	
	s = strstr (o->req + debut, "\r\n\r\n");
	if (!s) return (s - o->req);
	return -1;
}

int chercher_fin_requete (Slot *o, int debut){
	for (int i = debut; o->req[i]; i++)
		if ( ((o->req[i] == '\n') && (o->req[i++] == '\n')) ||
				((o->req[i] == '\r') && (o->req[i+1] == '\n') && (o->req[i+2] == '\r') && (o->req[i+3] == '\n')))
			return i;
	return -1;
}

char *get_http_error_message (Code_reponse code){
	switch (code) {
		case C_OK : return "ok";
		case C_BAD_REQUEST : return "Bad request";
		case C_NOT_FOUND : return "Not found";
		case C_METHOD_UNKNOWN : return "Method unknown";
	}
	return "Other error";
}

char* get_extension(char* chemin){
	char* extension = strrchr(chemin,'.');
	if (!extension || extension == chemin) return "";
	return extension+1;
}

void chercher_type_mime(Infos_entete* ie){
	char* extension = get_extension(ie->url);
	if (strcmp(extension,"html") == 0)
		strcpy(ie->type_mime, "text/html");
	else if (strcmp(extension,"css") == 0)
		strcpy(ie->type_mime, "text/css");
	else if (strcmp(extension,"png") == 0)
		strcpy(ie->type_mime, "image/png");
	else if (strcmp(extension,"jpeg") == 0)
		strcpy(ie->type_mime, "image/jpg");
	else
		strcpy(ie->type_mime, "application/octet-stream");
	printf("\n\ntype mime : %s\n", ie->type_mime);
}

int proceder_lecture_fichier(Slot* o){
	printf("Préparation de la lecture du fichier...\n");
	o->fic_pos = 0;

	o->fic_len = bor_read_str(o->fic_fd, o->fic_bin, FICSIZE);
	printf("Taille de chaine lue : %d\n", o->fic_len);
	if (o->fic_len > 0){
		o->etat = E_ENVOYER_FICHIER;
		return 1;
	}
	printf("Fichier lu !\n");
	return -1;
}

int proceder_envoi_fichier(Slot* o){
	printf("\n\nPréparation de l'envoi du fichier %d ...\n",o->fic_fd);
	int k = bor_write(o->soc, o->fic_bin+o->fic_pos, o->fic_len-o->fic_pos);
	if (k > 0) 
		o->fic_pos += k;
	printf("%d/%d\n", o->fic_pos,o->fic_len);
	if (o->fic_pos < o->fic_len)
		return 1;
	o->etat = E_LIRE_FICHIER;
	return 1;
}

int preparer_fichier (Slot *o, Infos_entete *ie){
	sscanf(ie->url, "%[^?]", ie->chemin);
	printf("Chemin trouvé : %s\n", ie->chemin);
	if (ie->chemin[strlen(ie->chemin)-1] == '/'){
		snprintf(ie->url, strlen(ie->url)+1+10, "%s%s", ie->url, "index.html");
		snprintf(ie->chemin, strlen(ie->chemin)+1+10, "%s%s", ie->chemin, "index.html");		
	}
	printf("Chemin trouvé : %s\n", ie->chemin);
	o->fic_fd = open(ie->chemin, O_RDONLY);
		
	if (o->fic_fd < 0){
		perror("open");
		return -1;
	}
	return 0;
}

Id_methode get_id_method (char *methode){
	if(!strcasecmp(methode, "GET")) return M_GET;
	if(!strcasecmp(methode, "TRACE")) return M_TRACE;
	return M_NONE;
}

int isGoodFormat(char* version){
	return (strcasecmp(version,"HTTP/1.0") || 
			strcasecmp(version,"HTTP/1.1"));
}

void analyser_requete (Slot *o, Infos_entete *ie){
	sscanf(o->req,"%s /%s %s\n", ie->methode, ie->url, ie->version);
	if (!isGoodFormat(ie->version))
		ie->code_rep = C_BAD_REQUEST;
	else if (get_id_method(ie->methode) == M_NONE)
		ie->code_rep = C_METHOD_UNKNOWN;
	else if (get_id_method(ie->methode) == M_TRACE){
		ie->id_meth = M_TRACE;
		ie->code_rep = C_OK;		
	}
	else if (get_id_method(ie->methode) == M_GET){
		ie->id_meth = M_GET;
		printf("Méthode GET : Cherche à récupérer %s...\n", ie->url);
		if (preparer_fichier(o,ie) < 0){
			ie->code_rep = C_NOT_FOUND;
			ie->file_size = 0;
		}
		else{	
			chercher_type_mime(ie);
			ie->code_rep = C_OK;
			struct stat stats;
			fstat(o->fic_fd,&stats);
			ie->file_size = stats.st_size;
		}
	}
}

void preparer_reponse (Slot *o, Infos_entete *ie){
	printf("Préparation de la réponse...\n");
	char* code_rep = get_http_error_message(ie->code_rep);
	char* date;
	char* content_type;
	char content_length[32];
	char body [2048];	
	
	time_t t = time(NULL);
	struct tm *tm;
	tm = localtime(&t);
	date = asctime(tm);
	switch (ie->code_rep){
		case C_OK:
			switch (ie->id_meth){
				case M_GET:
					content_type = ie->type_mime;
					sprintf(content_length, "Content-Length: %lu\r\n", ie->file_size);
					sprintf(body, "%s", o->rep);
				break;
				
				case M_TRACE:
					content_type = "message/http";
					sprintf(body, "%s", o->req);
				break;
				
				default:
					break;
			}
			break;
		default:
			content_type = "text/html";
			sprintf(body, "<html><body><h1>"
				   "Erreur : %s !"
				   "</h1></body></html>\r\n", code_rep);
			break;	
	}
	snprintf(o->rep, REPSIZE,
	"HTTP/1.1 %s\r\n"
	"Date: %s"
	"Server: serweb2\r\n"
	"Connection: close\r\n"
	"%s"
	"Content-Type: %s\r\n\r\n"
	"%s",code_rep,date,content_length,content_type,body);
}

int proceder_lecture_requete (Slot *o){
	int prec_pos = o->req_pos;
	int k = lire_suite_requete (o);
	if (k <= 0) return -1;
	int debut = prec_pos - 3;
	if (debut < 0) debut = 0;
	o->fin_entete = chercher_fin_requete (o, debut);	
	if (o->fin_entete < 0){
		printf ("serveur[%d]:requete incomplete\n", o->soc);
		return 1;
	}
	printf ("serveur[%d]: recu requete %s\n", o->soc, o->req);
	Infos_entete ie;
	analyser_requete (o, &ie);
	preparer_reponse (o, &ie);
	
	o->etat = E_ECRIRE_REPONSE;
	return 1;
}

int proceder_ecriture_reponse (Slot *o){
	int k = ecrire_suite_reponse (o);
	if (k < 0) return -1;
	if (o->rep_pos < (int) strlen (o->rep)){
		printf ("serveur[%d]:requete incomplete\n", o->soc);
		return 1;
	}
	printf("proceder_ecriture_reponse\n");
	if (o->fic_fd != -1){
		o->etat = E_LIRE_FICHIER;
		return 1;
	}
	return -1;
}

void traiter_slot_si_eligible (Slot *o, fd_set *set_read, fd_set *set_write){
	if (slot_est_libre (o)) return;
	int k = 1;
	switch (o->etat) {
		case E_LIRE_REQUETE:
			if (FD_ISSET (o->soc, set_read))
				k = proceder_lecture_requete (o);
			break;

		case E_ECRIRE_REPONSE:
			if (FD_ISSET (o->soc, set_write))
				k = proceder_ecriture_reponse (o);
			break;

		case E_LIRE_FICHIER:
			if(FD_ISSET(o->fic_fd, set_read))
				k = proceder_lecture_fichier(o);
			break;

		case E_ENVOYER_FICHIER:
			if(FD_ISSET(o->soc,set_write))
				k = proceder_envoi_fichier(o);
			break;

		default:
			break;
	}
	if (k <= 0){
		printf ("Serveur[%d] : libération slot\n", o->soc);
		liberer_slot (o);
	}
	printf("traiter_slot_si_eligible\n");
}

void inserer_fd (int fd, fd_set *set, int *maxfd){
	FD_SET (fd, set);
	if (*maxfd < fd)
		*maxfd = fd;
	printf("inserer_fd\n");
}

void preparer_select (Serveur *ser, int *maxfd, fd_set *set_read, fd_set *set_write) {
	FD_ZERO (set_read);
	FD_ZERO (set_write);
	
	*maxfd = -1;
	inserer_fd(ser->soc_ec, set_read, maxfd);
	for (int i = 0; i < SLOTS_NB; i++){
		Slot *o = &ser->slots[i];
		if (slot_est_libre (o))
			continue;
		switch (o->etat) {
			case E_LIRE_REQUETE:
				inserer_fd(o->soc, set_read, maxfd);
				break;
			case E_ECRIRE_REPONSE:
				inserer_fd(o->soc, set_write, maxfd);
				break;
			case E_LIRE_FICHIER:
				inserer_fd(o->fic_fd, set_read, maxfd);
				break;
			case E_ENVOYER_FICHIER:
				inserer_fd(o->soc, set_write, maxfd);
				break;
			default:
				break;
		}
	}
}

int faire_scrutation (Serveur* ser){
	int maxfd;
	fd_set set_read, set_write;
	preparer_select(ser, &maxfd, &set_read, &set_write);
	int res = select(maxfd+1, &set_read, &set_write, NULL, 0);
	if (res < 0){
		perror("select");
		return -1;
	}
	if (res == 0){
		Slot *o = bor_timer_data ();
		liberer_slot (o);
	}
	
	if (FD_ISSET(ser->soc_ec, &set_read)){
		int k = accepter_connexion(ser);
		if (k < 0)
			return -1;
	}
	for (int i = 0 ; i < SLOTS_NB ; i++){
		Slot* o = &ser->slots[i];
		traiter_slot_si_eligible(o,&set_read,&set_write);
	}
	return 1;
}

int boucle_princ = 1;

void capter_sigint(int sig){
	(void)sig;
	boucle_princ = 0;
}

int main (int argc, char** argv){
	bor_signal (SIGINT, capter_sigint, 0);
	if (argc < 2){
		printf("usage : %s numéro de port\n", argv[0]);
		exit(1);
	}
	Serveur *ser = malloc(sizeof(Serveur));
	int port = atoi(argv[1]);
	demarrer_serveur (ser, port);
	while (boucle_princ){
		if (faire_scrutation(ser) < 0)
			break;
	}
	fermer_serveur (ser);
	free(ser);
	return 0;
}