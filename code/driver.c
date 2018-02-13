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
	printf("HELLO THERE\n");
	//my_pthread_mutex_lock(&mutex);
	/*while(1==1){

	}*/
}

void func2(int x){
	//while(1==1);
	my_pthread_mutex_lock(&mutex);
	printf("HELLO Vrooooo %d\n", x);
	my_pthread_mutex_unlock(&mutex);
	//while(1==1);	
	//my_pthread_mutex_unlock(&mutex);	
	
}
int main(int argc, char ** argv){
	init();

	my_pthread_mutex_init(&mutex, NULL);
	my_pthread_t  thread;
	my_pthread_t  other;
	/*my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);*/
	my_pthread_mutex_lock(&mutex);
	//delay(1);
	my_pthread_create(other,NULL,(void*)&func2,5);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	

	my_pthread_mutex_unlock(&mutex);	
	//my_pthread_create(thread,NULL,(void*)&func1,NULL);
	
	printf("We done\n");
	while(1==1);
	//
	//removeThread();
}
