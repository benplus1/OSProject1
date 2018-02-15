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
int func1(){
	//my_pthread_mutex_lock(&mutex);
	//my_pthread_mutex_unlock(&mutex);
//	while(1==1);
	/*while(1==1){

	}*/
	int i=0;
	while(i<1000){
		i++;
	}
	//delay(100);
	printf("HELLO THERE\n");
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
	//while(1==1);
	//my_pthread_mutex_lock(&mutex);
	while(x>0){
		printf("HELLO Vrooooo %d\n", x);
		x--;
	}

	//while(1==1);	
	
}
int main(int argc, char ** argv){
	init();

	my_pthread_mutex_init(&mutex, NULL);
	//my_pthread_mutex_lock(&mutex);
	int i=0;
	for(i=0;i<10;i++){
		my_pthread_t  thread;
		my_pthread_create(thread,NULL,(void*)&func1,i);
		int * x;
		my_pthread_join(thread,&x);
		printf("%d\n", *x);
	}
	//my_pthread_mutex_unlock(&mutex);	
	//my_pthread_create(thread,NULL,(void*)&func1,NULL);
	int res;	
	//my_pthread_join(other,&res);
	//my_pthread_mutex_lock(&mutex);
	printf("We done\n");
	while(1==1);
	//
	//removeThread();
}
