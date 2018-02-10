#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<ucontext.h>
#include<unistd.h>
#include<sys/time.h>
#include "my_pthread.c"

void func1(){
	printf("HELLO THERE\n");
	/*while(1==1){
		
	}*/
}

void func2(){
	printf("HELLO Vrooooo\n");
	//while(1==1);
}
int main(int argc, char ** argv){
	init();
	my_pthread_mutex_t mutex;
	my_pthread_mutex_init(&mutex, NULL);
	my_pthread_t * thread;
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	my_pthread_create(thread,NULL,(void*)&func1,NULL);
	printf("We done\n");
	//while(1==1);	
	//removeThread();
}
