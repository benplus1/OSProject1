#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<ucontext.h>
#include<unistd.h>
#include<sys/time.h>
#include "my_pthread.c"

void my_function(){
	printf("HELLO THERE\n");
	while(1==1);
}

int main(int argc, char ** argv){
	init();
	my_pthread_t * thread;
	my_pthread_create(thread,NULL,(void*)&my_function,NULL);
	my_pthread_create(thread,NULL,(void*)&my_function,NULL);
	
	//removeThread();
}
