#include <bor-util.h>

void bor_set_sockaddr_in(int port, uint32_t ipv4, struct  sockaddr_in * sa){
	sa->sin_family=AF_INET;
	sa->sin_port=htons(port);
	sa->sin_addr.s_addr=htonl(ipv4);
}

int bor_create_socket_in(
	int type, //sock_STREAM ou sock_DGRAM
	int port, //0 pour libre
	struct sockaddr_in *sa){

	int soc=socket(AF-INET,type,0);
	if(soc < 0){
		perror(__func__);
		return -1;
	}
	bor_set_sockaddr_in(port, INADDR_ANY, sa);
	if(bor_bind_in(soc,sa)<0){
		close(soc);
		return -1;
	}
	if(bor_get_sockname_in(soc,sa)<0){
		close(soc);
		return -1;
	}
	printf("%s : port %d ouvert\n",__func__,ntohs(sa->sin_port));
	return soc;
}

int bor_resolve_address_in(const char *host,int port, struct  sockaddr_in * sa){
	sa->sin_family=AF_INET;
	sa->sin_port=htons(port);
	struct hostent *hp;
	hp=gethostbyname(host);
	if(hp == NULL){
		herror(__func__);
		return -1;
	}
	memcpy(&sa->sin_addr.s_addr,hp->h_addr,hp->h_length);
	return 0;
}

