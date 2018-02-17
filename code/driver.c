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
void delay(int milliseconds)
{
	long pause;
	clock_t now,then;

	pause = milliseconds*(CLOCKS_PER_SEC/1000);
	now = then = clock();
	while( (now-then) < pause )
		now = clock();
}

int func2(int x){
	int y=0;
	int i=1;
	my_pthread_mutex_lock(&mutex);
	printf("Length delay %d\n",x);
	printf("RES%d\n",test);
	my_pthread_mutex_unlock(&mutex);
	return x+6;
}
int main(int argc, char ** argv){
	init();
	my_pthread_mutex_init(&mutex, NULL);
	int k;
	my_pthread_t * arr=(my_pthread_t *)malloc(sizeof(my_pthread_t)*5);
	my_pthread_mutex_lock(&mutex);
	for (k=0;k<5;k++){
		my_pthread_t  thread;
		int * i=12;
		my_pthread_create(&thread,NULL,(void*)&func2,k);
	}
	delay(5000);
	printf("Unlocking mutex\n");
	//my_pthread_mutex_destroy(&mutex);
	//while(1==1);
}
