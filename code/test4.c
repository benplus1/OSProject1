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
int func1(int y){
	printf("I am %d\n",y);
	return 2;
}

int func2(int x){
	int i=200;
	int * arr[20000];
	arr[i] = (int *) malloc(sizeof(int)*4096*200);
	*arr[i]=30;
	printf("Param T1: %d %lx\n",*arr[i],arr[i]);
	return x+6;
}


int main(int argc, char ** argv){
	init_memory();
	my_pthread_t t1,t2,t3;
	int w=4;
	int i=200;
	int * arr[20000];
	int j;
	for (j=0;j<10;j++){
		arr[j] = (int *) malloc(sizeof(int)*4096*20);
		if(arr[j]==NULL){
			printf("Freeing memory \n");
			free(arr[j-1]);
			arr[j]=malloc(sizeof(int)*4096*200);
		}
		*arr[j]=19;
	}	

	for(j=0;j<10;j++){

		pthread_create(&t1,NULL,(void*)&func2,w);
		pthread_join(t1,NULL);	
		printf("Param T0: %d %lx\n",*arr[0],arr[0]);

	}
	//pthread_create(&t2,NULL,(void*)&func2,w);
}



