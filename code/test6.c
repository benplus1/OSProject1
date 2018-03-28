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
int test=0;
int * arr;
int func2(int x){
	printf("Param T1: %d %lx\n",arr[10],&arr[10]);
	
	return x+6;
}


int main(int argc, char ** argv){
	init_memory();
	my_pthread_t t1,t2,t3;
	arr = (int *) shalloc(4096*2);
	int j;
	for (j=0;j<200;j++){
		arr[j]=98;
	}	
	int w=12;
	pthread_create(&t1,NULL,(void*)&func2,w);
	pthread_join(t1,NULL);	
	printf("Param T0: %d %lx\n",arr[10],&arr[10]);
}



