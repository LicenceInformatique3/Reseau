/*int bor_timer_add(			//calcule date expiratoire
	unsigned long delay,	//en ms 
	void * data){
	if(bor_timer_nb>=BOR_TIMER_MAX){
		fprintf(stderr, "bor_timer_add  : erreur , trop de timers\n", );
		return -1;
	}
	struct timeval t;
	gettimeofday(&t,NULL);
	t.tv_usec+= delay*1000;
	if (t.tv_usec >1000000)	{
		t.tv_sec+=t.tv_usec/1000000;
		t.tv_usec=t.tv_usec%1000000;
	}

	//recherche dichotomique
	int m1=0,m2=bor_timer_nb;
	while(m2-m1>0){
		int mid =(m1+m2)/2
		struct timeval *mt=&bor_timer_list[mid].expiration;
		if (mt->tv_sec<t.tv_sec || (mt->tv_sec==t.tv_sec && mt->tv_usec<t.tv_usec)){
			m1=mid+1;
		}
		else
			m2=mid;
	}
	//on insère en m1=m2
	//on décale [m1 ... mb[ droite
	if(m1<bor_timer_nb){
		memmove(//dst,src,taille
			bor_timer_list+m1+1,bor_timer_list,(nb-m1)*sizeof(bor_timer_struct))
	}
	bor_timer_nb++;
	bor_timer_struct *ts=&bor_timer_list[m1];
	ts->handle =bor_timer_uniq++;
	ts->data=data;
	ts->expiration=t;
	return ts->handle;
}


void bor_timer_remove(int handle){
	if(handle<0)
		return;
	for (int i = 0; i < bor_timer_nb; ++i){
		if (bor_timer_list[i].handle==handle){
			memmove(bor_timer_list+i,bor_timer_list+i+1,(bor_timer_nb-i-1)*sizeof(bor_timer_struct));
			bor_timer_nb--;
			return;
		}
	}
}

struct timeval * bor_timer_delay(){
	static struct timeval t;
	if(bor_timer_nb==0){
		return NULL;
	}
	gettimeofday(&t,NULL);
	t.tv_sec=bor_timer_list[0].expiration.tv_sec-t.tv_sec;
	t.tv_usec=bor_timer_list[0].expiration.tv_usec-t.tv_usec;
	if(t.tv_usec<0){
		t.tv_sec--;
		t.tv_usec+=1000000;
	}
	if (t.tv_usec<0){
		t.tv_sec=t.tv_usec=0;	//on est en retard
	}
	printf("Timeout dans %ds %dµs\n",t.tv_sec,t.tv_usec );
	return &t;
}

int bor_timer_handle(){
	if(bor_timer_nb<=0)
		return -1;
	return bor_timer_list[0].handle;
}

void *bor_timer_data(){
	if(bor_timer_nb<=0)
		return NULL;
	return bor_timer_list[0].data;
}
*/
#include "bor-util.h"
#include "bor-timer.h"

int main(){

	int boucle_princ=1;
	int h1=bor_timer_add(2000,NULL);
	int h2=bor_timer_add(5000,NULL);
	int h3=bor_timer_add(10000,NULL);
	int h4=bor_timer_add(20000,NULL);

	while(boucle_princ){
		fd_set set;
		FD_ZERO(&set);
		int res=select(0,&set,NULL,NULL,bor_timer_delay());
		
		if(res<0){
			if(errno==EINTR)
				continue;
			perror("select");
			break;
		}

		if(res==0){
			int handle=bor_timer_handle();
			if(handle==h1){
				printf("timer 1\n");
				h1=bor_timer_add(2000,NULL);
			}
			else if(handle==h2){
				printf("timer 2\n");
				h2=bor_timer_add(5000,NULL);
			}
			else if(handle==h3){
				printf("timer 3\n");
				h3=bor_timer_add(10000,NULL);
			}
			else if(handle==h4){
				printf("timer 4\n");
				h4=bor_timer_add(20000,NULL);
				boucle_princ=0;
			}
			bor_timer_remove(handle);
		}
	}
}