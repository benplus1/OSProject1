#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<ucontext.h>
#include<unistd.h>
#include<sys/time.h>
#include<time.h>
#include "my_pthread.c"
my_pthread_mutex_t mutex;
void func1(){
	my_pthread_mutex_lock(&mutex);
	printf("HELLO THERE\n");
	my_pthread_mutex_unlock(&mutex);
//	while(1==1);
	/*while(1==1){

	}*/
}

void func2(int x){
	//while(1==1);
	//my_pthread_mutex_lock(&mutex);
	printf("HELLO Vrooooo %d\n", x);

	//while(1==1);	
	
}
int main(int argc, char ** argv){
	init();

	my_pthread_mutex_init(&mutex, NULL);
	my_pthread_t  thread;
	my_pthread_t  other;
	my_pthread_t  other2;
	my_pthread_t  other3;
	/*my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);*/
	//delay(1);
	my_pthread_create(other,NULL,(void*)&func2,5);
	my_pthread_create(other2,NULL,(void*)&func2,2);
	my_pthread_create(other3,NULL,(void*)&func2,6);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);

	//my_pthread_mutex_unlock(&mutex);	
	//my_pthread_create(thread,NULL,(void*)&func1,NULL);
	int res;	
	my_pthread_join(thread,&res);
	my_pthread_mutex_lock(&mutex);
	printf("We done\n");
	//while(1==1);
	//
	//removeThread();
}
