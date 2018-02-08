// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"

lq ** scheduler;

int mem=4096;

void init(){
	printf("Vro\n");
	scheduler=(lq **) malloc(sizeof(lq *)*3);
	(*(scheduler))=(lq *)malloc(sizeof(lq));	
	(*(scheduler))->front=NULL;
	(*(scheduler+1))=(lq *)malloc(sizeof(lq));	
	(*(scheduler+1))->front=NULL;
	(*(scheduler+2))=(lq *)malloc(sizeof(lq));	
	(*(scheduler+2))->front=NULL;
}

ucontext_t * init_context(void* func){
	ucontext_t * t=(ucontext_t *) malloc(sizeof(ucontext_t));
	getcontext(t);
	t->uc_link=0;
	t->uc_stack.ss_sp=malloc(mem);
	t->uc_stack.ss_size=mem;
	t->uc_stack.ss_flags=0;
	makecontext(t,func, 0);
	return t;
}

tcb * init_tcb(ucontext_t * context, my_pthread_t * tid){
	tcb * curr=(tcb *)malloc(sizeof(tcb *));
	curr->context=context;
	curr->tid=tid;
	return curr;
}

void enqueue(tcb * curr, int priority){
	lq * currlq =scheduler[priority];
	tcb * ptr=currlq->front;
	if(ptr==NULL){
		ptr=curr;
	}else{
		while(ptr->right!=NULL){
			ptr=ptr->right;
		}
		ptr->right=curr;
		curr->left=ptr;
	}
	printf("Enqueued\n");
}


/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	ucontext_t * my_context = init_context(function);
	//setcontext(&my_context);
	tcb * curr= init_tcb(my_context, thread);
	enqueue(curr, 0);	
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	return 0;
};
