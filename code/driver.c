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

func2(int x){
	int y=0;
	int i=1;
	delay(4000);
	printf("Iteration %d\n",i);
	printf("Result of unlocking:%d\n",y);
}
int main(int argc, char ** argv){
	init();
	my_pthread_mutex_init(&mutex, NULL);
	my_pthread_t  thread;
	int i=12;
	my_pthread_create(&thread,NULL,(void*)&func2,i);
	printf("We done\n");
}
