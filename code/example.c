#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<ucontext.h>
#include<unistd.h>
#include<sys/time.h>

int mem=64000;
ucontext_t t1,t2,t3,t4;
struct itimerval timer;
void fn1(){
	printf("This is f1\n");
	//setcontext(&t3);
}

void pthread_exit(){
	setcontext(&t3);
}

void fn2(){
	printf("This is from 2\n");
	//setcontext(&t3);
}

void sig_handler(){
	printf("Caught signal");
	setcontext(&t3);	
}

int main(int argc, char ** argv){
	getcontext(&t3);
	while(signal(SIGVTALRM,(void *)&sig_handler)==SIG_ERR);
	timer.it_value.tv_sec=25/1000;
	timer.it_value.tv_usec=25;
	timer.it_interval=timer.it_value;
	while(setitimer(ITIMER_VIRTUAL,&timer,NULL)==-1);
	
	getcontext(&t4);
	t4.uc_link=0;
	t4.uc_stack.ss_sp=malloc(mem);
	t4.uc_stack.ss_size=mem;	
	t4.uc_stack.ss_flags=0;
	makecontext(&t4,(void *)&pthread_exit, 0);
	
	getcontext(&t1);
	t1.uc_link=&t4;
	t1.uc_stack.ss_sp=malloc(mem);
	t1.uc_stack.ss_size=mem;	
	t1.uc_stack.ss_flags=0;
	
	makecontext(&t1,(void *)&fn1,0);
	swapcontext(&t3,&t1);
	printf("To the next thread\n");
	
	getcontext(&t2);
	t2.uc_link=&t4;
	t2.uc_stack.ss_sp=malloc(mem);
	t2.uc_stack.ss_size=mem;	
	t2.uc_stack.ss_flags=0;
	makecontext(&t2,(void *) &fn2,0);
	swapcontext(&t3,&t2);
	//while(0==0);
	//swapcontext(&t3,&t1);
}


