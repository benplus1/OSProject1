
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"

const int RUNNING=0;
const int WAITRES=1;
const int READY=2;
const int LOCKING=3;
const int TERMINATED=4;
const int WAITLOCK = 5;

//xd
mutexP * mutexPool;
lq ** scheduler;

sigset_t blockSet;
sigset_t emptySet;



int cyclesLeft=0;
int numLevels=3;
int isHandling=0;
int didStart=0;
struct itimerval timer;
int mem=4096;
//ucontext_t * tailFunc;

tcb * currThread=NULL;




void sig_handler(){
	//printf("Caught signal\n");
	drop(currThread);
	schedule();
}

void * func(void *(*function)(void*), void * arg, tcb * thread){
	void * value_ptr=function(arg);
	thread->res=value_ptr;		
}

void schedule(){
	if(!isHandling){
		if(cyclesLeft!=0){
			cyclesLeft--;
			while(sigprocmask(SIG_UNBLOCK, &blockSet, NULL)<0);
			return;
		}
		printf("Scheduling\n");
		isHandling=1;
		tcb * next_tcb;
		int i=0;
		for(i=0;i<numLevels;i++){
			next_tcb=scheduler[i]->front; //fix this to take states and multilevel priority queue
			while(next_tcb!=NULL){
				if(next_tcb->state==READY||next_tcb->state==LOCKING||next_tcb->state==RUNNING){
					break;
				}
				next_tcb=next_tcb->right;
			}
			if(next_tcb!=NULL){
				break;
			}
		}

		/*if(next_tcb->state==WAITRES){
		  printf("SHEET%d\n", next_tcb->state);
		  }*/
		//while(setitimer(ITIMER_VIRTUAL,&timer,NULL)==-1);
		if(next_tcb!=NULL){
			ucontext_t * next=next_tcb->context;
			//printf("Swapping Context\n");
			cyclesLeft=next_tcb->priority;	
			if(currThread!=NULL){
				ucontext_t * prevContext=currThread->context;
				currThread=next_tcb;
				while(sigprocmask(SIG_UNBLOCK, &blockSet, NULL)<0);
				isHandling=0;
				swapcontext(prevContext, next );
			}else{
				isHandling=0;
				currThread=next_tcb;
				while(sigprocmask(SIG_UNBLOCK, &blockSet, NULL)<0);
				setcontext(next);
			}
		}else{
			//printf("No new threads to run");
			isHandling=0;
			while(1==1);
			while(sigprocmask(SIG_UNBLOCK, &blockSet, NULL)<0);

		}
	}
	while(sigprocmask(SIG_UNBLOCK, &blockSet, NULL)<0);

}

ucontext_t * init_tail(){
	ucontext_t * tailFunc=malloc(sizeof(ucontext_t));
	getcontext(tailFunc);
	tailFunc->uc_link=0;
	tailFunc->uc_stack.ss_sp=malloc(mem);
	tailFunc->uc_stack.ss_size=mem;
	tailFunc->uc_stack.ss_flags=0;
	makecontext(tailFunc,(void *)&my_pthread_exit, 0);
	return tailFunc;
}

ucontext_t * init_context(void* func, void* arg){
	ucontext_t * t=(ucontext_t *) malloc(sizeof(ucontext_t));
	while(getcontext(t)==-1);
	ucontext_t * tailFunc=init_tail();
	t->uc_link=tailFunc;
	t->uc_stack.ss_sp=malloc(mem);
	t->uc_stack.ss_size=mem;
	t->uc_stack.ss_flags=0;
	makecontext(t,func, 1, arg);
	return t;
}


tcb * init_tcb(my_pthread_t * tid, ws * currArgs, void * func){
	tcb * curr=(tcb *)malloc(sizeof(tcb));
	curr->right=NULL;
	curr->left=NULL;
	curr->tid=tid;
	curr->state=READY;
	curr->priority=0;
	curr->waiting=NULL;
	curr->res=malloc(sizeof(void**));
	curr->args=currArgs;
	curr->func=func;
	if (curr->func != NULL) {
		(curr->args)->tcb=curr;
		curr->context=init_context(curr->func, curr->args);
	}
	else {
		ucontext_t * prevContext=malloc(sizeof(ucontext_t));
		getcontext(prevContext);
		curr->context=prevContext;
	}
	return curr;
}

wn * init_wn(tcb * curr){
	wn * waitNode=(wn *)malloc (sizeof(wn));
	waitNode->curr=curr;
	waitNode->next=NULL;
	return waitNode;
}

tcb * find(my_pthread_t tid){
	int i;
	for(i=0;i<numLevels;i++){
		tcb * ptr=scheduler[i]->front; //fix this to take states and multilevel priority queue
		while(ptr!=NULL){
			if(*(ptr->tid)==tid){
				return ptr;
			}
			ptr=ptr->right;
		}
	}
	return NULL;
}

void * wrapper(ws * currArgs) {
	void *(*func)(void*) = currArgs->func;
	if (func != NULL) {
		void ** ptr=(currArgs->tcb)->res;
		int res=func(currArgs->args);
		*ptr=res;		
		//printf("populated\n");
	}
}

void drop(tcb * curr){
	removeThread(curr);
	if(curr->priority<(numLevels-1)){
		curr->priority++;
	}
	else if(curr->priority == numLevels-1) {
		curr->priority++;
	}
	enqueue(curr,curr->priority);
}

void enqueue(tcb * curr, int priority){
	lq * currlq =scheduler[priority];
	tcb * ptr=currlq->front;
	if(ptr==NULL){
		currlq->front=curr;
	}else{
		while(ptr->right!=NULL){
			ptr=ptr->right;
		}
		ptr->right=curr;
		curr->left=ptr;
	}
	//printf("Enqueued\n");
}

void removeThread(tcb * curr){
	tcb * left=curr->left;
	tcb * right=curr->right;
	if(curr->left==NULL){
		scheduler[curr->priority]->front=curr->right;
	}
	if(left!=NULL){
		left->right=right;
	}
	if(right!=NULL){
		right->left=left;
	}
	curr->left=NULL;
	curr->right=NULL;
	//curr->state=TERMINATED;
}

void init(){
	printf("Vro\n");
	//Init scheduler
	scheduler=(lq **) malloc(sizeof(lq *)*(numLevels+1));
	int i;
	for(i=0;i<numLevels+1;i++){		
		(*(scheduler+i))=(lq *)malloc(sizeof(lq));
		(*(scheduler+i))->front=NULL;
	}

	ucontext_t * prevContext=malloc(sizeof(ucontext_t));
	my_pthread_t * prev_tid;
	getcontext(prevContext);
	prevContext->uc_link=init_tail();
	currThread=init_tcb(prev_tid, NULL, NULL);
	enqueue(currThread,currThread->priority);
	//Init tail context for every thread

	//init mutexpool
	mutexPool = (mutexP *) malloc(sizeof(mutexP));

	//init alarm
	while(signal(SIGVTALRM,(void *)&sig_handler)==SIG_ERR);
	timer.it_value.tv_sec=25/1000;
	timer.it_value.tv_usec=250;
	timer.it_interval=timer.it_value;
	while(setitimer(ITIMER_VIRTUAL,&timer,NULL)==-1);

	//sigprocmask( SIGVTALRM,&set,NULL);
	sigemptyset(&emptySet);
	sigemptyset(&blockSet);
	sigaddset(&blockSet, SIGVTALRM);

	printf("Done init\n");
	//while(1==1);
}



/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	sigprocmask(SIG_SETMASK, &blockSet,NULL);
	//ucontext_t * my_context = init_context(function,  arg);
	//setcontext(&my_context);
	ws * currArgs = (ws *) malloc(sizeof(ws));
	currArgs->func = function;
	currArgs->args = arg;
	tcb * curr= init_tcb(thread, currArgs, &wrapper);


	enqueue(curr, 0);
	if(!didStart){
		didStart=1;
		schedule();
	}else{
		sigprocmask(SIG_UNBLOCK, &blockSet,NULL);

	}
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	cyclesLeft=0;
	sig_handler();
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	sigprocmask(SIG_SETMASK, &blockSet,NULL);
	printf("Exiting\n");


	currThread->state=TERMINATED;
	removeThread(currThread);
	//my_pthread_yield();

	//Update waiting threads
	while(currThread->waiting!=NULL){
		currThread->waiting->curr->state=READY;
		currThread->waiting=currThread->waiting->next;
	}
	//Adding to dead queue;
	lq * deadQueue=*(scheduler+numLevels);
	currThread->right=deadQueue->front;
	deadQueue->front=currThread;
	currThread=NULL;
	cyclesLeft=0;
	schedule();

};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {	
	sigprocmask(SIG_SETMASK, &blockSet,NULL);	
	tcb * target= find(thread);
	if(target==NULL||target->state==TERMINATED) return -1;
	currThread->state=WAITRES;
	wn * waitNode=init_wn(currThread);		
	waitNode->next=target->waiting;
	target->waiting=waitNode;
	*value_ptr=(target->res);
	my_pthread_yield();	
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	sigprocmask(SIG_SETMASK, &blockSet,NULL);
	mutex->id = mutexPool->size;
	mutex->currT = NULL;
	mutex->waiting = NULL;
	int i;
	++(mutexPool->size);
	my_pthread_mutex_t* ptr = mutexPool->front;
	if (ptr == NULL) {
		mutexPool->front = mutex;
	}else{
		while(ptr->next != NULL) {
			if(ptr->id==mutex->id){	
				sigprocmask(SIG_UNBLOCK, &blockSet,NULL);
				return 0;
			}
			ptr = ptr->next;
		}
		ptr->next = mutex;
	}

	my_pthread_yield();
	//sigprocmask(SIG_UNBLOCK, &blockSet, NULL);

	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	sigprocmask(SIG_SETMASK, &blockSet,NULL);
	if(mutex->currT==NULL){
		mutex->currT=currThread;
		currThread->state=LOCKING;
	}else{
		printf("Gotta wait\n");
		wn * ptr=mutex->waiting;
		wn * waitNode=init_wn(currThread);
		currThread->state=WAITLOCK;
		if(ptr==NULL){
			mutex->waiting=waitNode;
		}else{
			while(ptr->next!=NULL){
				ptr=ptr->next;
			}
			ptr->next=waitNode;
		}
	}
	//printf("lock ops\n");
	my_pthread_yield();
	//sigprocmask(SIG_UNBLOCK, &blockSet, NULL);
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	//printf("ulock status %d\n", currThread->state);
	sigprocmask(SIG_SETMASK, &blockSet,NULL);
	if(mutex->currT==NULL||mutex->currT->tid!=currThread->tid){
		//printf("Can't do that\n");
		//my_pthread_yield();
		sigprocmask(SIG_UNBLOCK, &blockSet,NULL);
		return -1;
	}
	wn * ptr= mutex->waiting;
	((mutex->currT))->state=READY;
	if(ptr!=NULL){
		((ptr->curr))->state=LOCKING;
		mutex->currT=ptr->curr;
		mutex->waiting=ptr->next;
	}else{
		mutex->currT=NULL;
	}
	my_pthread_yield();
	//sigprocmask(SIG_UNBLOCK, &blockSet, NULL);
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	sigprocmask(SIG_SETMASK, &blockSet,NULL);
	my_pthread_mutex_t* ptr = mutexPool->front;
	my_pthread_mutex_t * prev=NULL;
	while(ptr!= NULL) {
		if(ptr->id==mutex->id){
			if(prev!=NULL){
				prev->next=ptr->next;
			}else{
				mutexPool->front=NULL;
			}
			free(mutex);
			my_pthread_yield();
			return 0;
		}
		prev=ptr;
		ptr = ptr->next;
	}
	my_pthread_yield();
	//free(mutex);
	return -1;
};
