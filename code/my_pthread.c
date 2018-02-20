
// Author:	Yujie REN
// Date:	09/23/2017

// name: Hemanth Chiluka, Andrew Dos Reis, Benjamin Yang
// username of iLab: hkc33, ad1005, bty10
// iLab Server: adapter

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

int enter_mode=0;
int mode=0;
sigset_t blockSet;
sigset_t emptySet;

int been_inited=0;

int num_inverted=0;

int threadCount=0;
int cyclesLeft=0;
int numLevels=3;
int isHandling=0;
int didStart=0;
struct itimerval timer;
int memSize=4096;
//ucontext_t * tailFunc;

tcb * currThread=NULL;




void signal_handler(){
	//printf("Caught signal\n");
	drop(currThread);
	schedule();
}

void * func(void *(*function)(void*), void * arg, tcb * thread){
	void * value_ptr=function(arg);
	thread->res=value_ptr;
}

void checkSigHandler(){
	if(!(__atomic_load_n(&mode,0))){
		signal_handler();
	}
}

void schedule(){

	if(!isHandling){


		/*if(next_tcb->state==WAITRES){
		  printf("SHEET%d\n", next_tcb->state);
		  }*/
		//while(setitimer(ITIMER_VIRTUAL,&timer,NULL)==-1);

		if(cyclesLeft!=0){
			cyclesLeft--;
			__atomic_clear(&mode,0);
			return;
		}
		//printf("Scheduling\n");
		isHandling=1;
		tcb * next_tcb;
		int i=0;
		for(i=0;i<numLevels;i++){
			next_tcb=scheduler[i]->front; //fix this to take states and multilevel priority queue
			while(next_tcb!=NULL){
				if(next_tcb->state==READY||next_tcb->state==LOCKING||next_tcb->state==RUNNING){
					break;
				}
				else if (next_tcb->state==WAITLOCK) {
					next_tcb->wait_skips++;
					if (next_tcb->wait_skips > 10) {
						tcb * to_be_swapped = (next_tcb->mutex_id)->currT;
						swap(next_tcb, to_be_swapped);
						//next_tcb->wait_skips = 0;
						//printf("inverting priority: %d\n", num_inverted);
						num_inverted++;
						next_tcb=to_be_swapped;
						if (next_tcb->state==READY||next_tcb->state==LOCKING||next_tcb->state==RUNNING) {
							break;
						}
					}
				}
				next_tcb=next_tcb->right;
			}
			if(next_tcb!=NULL&&(next_tcb->state==READY||next_tcb->state==LOCKING||next_tcb->state==RUNNING)){
				break;
			}
		}
		if(next_tcb!=NULL&&(next_tcb->state==READY||next_tcb->state==LOCKING||next_tcb->state==RUNNING)){
			//printf("Thread id: %d State of entering thread %d\n",*(next_tcb->tid), next_tcb->state);
			fflush(stdout);
			ucontext_t * next=next_tcb->context;
			cyclesLeft=next_tcb->priority;
			if(currThread!=NULL){
				ucontext_t * prevContext=currThread->context;
				currThread=next_tcb;
				isHandling=0;
				__atomic_clear(&mode,0);
				while(swapcontext(prevContext, next )<0);
			}else{
				isHandling=0;
				currThread=next_tcb;
				__atomic_clear(&mode,0);
				while(setcontext(next)<0);
			}
		}else{
			//printf("No new threads to run");
			isHandling=0;
			//printf("Deadlock\n");
			//while(1==1);
			signal_handler();
			__atomic_clear(&mode,0);

		}
	}else{
		__atomic_clear(&mode,0);
	}
}

void swap(tcb * curr1, tcb * curr2) {
	tcb * c1_left = curr1->left;
	tcb * c1_right = curr1->right;
	int enteredSwap = 0;
	if (c1_right != NULL) {
		if (c1_right->tid==curr2->tid) {
			enteredSwap = 1;
			curr1->left = curr2;
			curr1->right = curr2->right;
			curr2->left = c1_left;
			curr2->right = curr1;

			if (curr2->left != NULL) {
				(curr2->left)->right = curr2;
			}
			if (curr1->right != NULL) {
				(curr1->right)->left = curr1;
			}
		}
	}
	if (c1_left != NULL) {
		if ((c1_left->tid==curr2->tid) && !enteredSwap) {
			enteredSwap = 1;
			curr1->right = curr2;
			curr1->left = curr2->left;
			curr2->right = c1_right;
			curr2->left = curr1;

			if (curr2->right != NULL) {
				(curr2->right)->left = curr2;
			}
			if (curr1->left != NULL) {
				(curr1->left)->right = curr1;
			}
		}
	}
	if (!enteredSwap) {
		curr1->left = curr2->left;
		curr1->right = curr2->right;
		curr2->left = c1_left;
		curr2->right = c1_right;

		if (curr1->left != NULL) {
			(curr1->left)->right = curr1;
		}
		if (curr1->right != NULL) {
			(curr1->right)->left = curr1;
		}
		if (curr2->left != NULL) {
			(curr2->left)->right = curr2;
		}
		if (curr2->right != NULL) {
			(curr2->right)->left = curr2;
		}
	}

	int curr1_front = 0;
	int curr2_front = 0;
	if(scheduler[curr1->priority]->front == curr1) {
		curr1_front = 1;
	}
	if(scheduler[curr2->priority]->front == curr2) {
		curr2_front = 1;
	}
	if(curr1_front) {
		scheduler[curr1->priority]->front = curr2;
	}
	if(curr2_front) {
		scheduler[curr2->priority]->front = curr1;
	}
	int swap_priority = curr1->priority;
	curr1->priority = curr2->priority;
	curr2->priority = swap_priority;
}

ucontext_t * init_tail(){
	ucontext_t * tailFunc=malloc(sizeof(ucontext_t));
	getcontext(tailFunc);
	tailFunc->uc_link=0;
	tailFunc->uc_stack.ss_sp=malloc(memSize);
	tailFunc->uc_stack.ss_size=memSize;
	tailFunc->uc_stack.ss_flags=0;
	makecontext(tailFunc,(void *)&my_pthread_exit, 0);
	return tailFunc;
}

ucontext_t * init_context(void* func, void* arg){
	ucontext_t * t=(ucontext_t *) malloc(sizeof(ucontext_t));
	while(getcontext(t)==-1);
	ucontext_t * tailFunc=init_tail();
	t->uc_link=tailFunc;
	t->uc_stack.ss_sp=malloc(memSize);
	t->uc_stack.ss_size=memSize;
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
	curr->num_drops=0;
	curr->mutex_id=NULL;
	curr->wait_skips=0;
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
	for(i=0;i<=numLevels;i++){
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
		//void * ptr=(currArgs->tcb)->res;
		void * res=func(currArgs->args);
		(currArgs->tcb)->res=res;
		//printf("populated\n");
	}
}

void drop(tcb * curr){
	removeThread(curr);
	curr->num_drops++;
	if( curr->priority==0 && curr->num_drops >= 25 && curr->num_drops <= 40) {
		curr->priority = 1;
		//printf("dropped to 1\n");
	}
	else if ( curr->priority==1 && curr->num_drops > 40 && curr->num_drops <= 60) {
		curr->priority = 2;
		//printf("dropped to 2\n");
	}
	else if ( curr->priority==2 && curr->num_drops > 60 && curr->num_drops <= 80) {
		curr->priority = 1;
		//printf("up to 1\n");
	}
	else if ( curr->priority==1 && curr->num_drops > 80) {
		curr->priority = 0;
		//printf("up to 0\n");
	}
	//if(curr->priority<(numLevels-1)){
	//	curr->priority++;
	//}
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
	//printf("Initializing Structs\n");
	//Init scheduler
	scheduler=(lq **) malloc(sizeof(lq *)*(numLevels+1));
	int i;
	for(i=0;i<numLevels+1;i++){
		(*(scheduler+i))=(lq *)malloc(sizeof(lq));
		(*(scheduler+i))->front=NULL;
	}

	ucontext_t * prevContext=malloc(sizeof(ucontext_t));
	my_pthread_t * prev_tid=(my_pthread_t *)malloc(sizeof(my_pthread_t));;
	*prev_tid=-1;
	getcontext(prevContext);
	prevContext->uc_link=init_tail();
	currThread=init_tcb(prev_tid, NULL, NULL);
	enqueue(currThread,currThread->priority);
	//Init tail context for every thread

	//init mutexpool
	mutexPool = (mutexP *) malloc(sizeof(mutexP));

	//init alarm
	while(signal(SIGVTALRM,(void *)&checkSigHandler)==SIG_ERR);
	timer.it_value.tv_sec=0;
	timer.it_value.tv_usec=250000;
	timer.it_interval=timer.it_value;
	while(setitimer(ITIMER_VIRTUAL,&timer,NULL)==-1);

	//sigprocmask( SIGVTALRM,&set,NULL);
	sigemptyset(&emptySet);
	sigemptyset(&blockSet);
	sigaddset(&blockSet, SIGVTALRM);

	//printf("Done initializing\n");
	//while(1==1);
}



/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	__atomic_test_and_set(&mode,0);
	if (!been_inited) {
		init();
		been_inited=1;
	}

	//ucontext_t * my_context = init_context(function,  arg);
	//setcontext(&my_context);
	ws * currArgs = (ws *) malloc(sizeof(ws));
	currArgs->func = function;
	currArgs->args = arg;
	*thread=threadCount;
	threadCount++;
	tcb * curr= init_tcb(thread, currArgs, &wrapper);


	enqueue(curr, 0);
	if(!didStart){
		didStart=1;
		schedule();
	}else{
		__atomic_clear(&mode,0);

	}
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {

	__atomic_test_and_set(&mode,0);
	if (!been_inited) {
		init();
		been_inited=1;
	}
	cyclesLeft=0;
	signal_handler();
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	if (!been_inited) {
		init();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	//printf("Exiting\n");


	currThread->state=TERMINATED;
	removeThread(currThread);
	//my_pthread_yield();

	//Update waiting threads
	while(currThread->waiting!=NULL){
		currThread->waiting->curr->state=READY;
		currThread->waiting=currThread->waiting->next;
	}
	if(value_ptr!=NULL){
		*(&value_ptr)=(currThread->res);
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
	if (!been_inited) {
		init();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	tcb * target= find(thread);
	if(target!=NULL&&value_ptr!=NULL){
		*value_ptr=&(target->res);
	}
	if(target==NULL||target->state==TERMINATED) {
		my_pthread_yield();
		return -1;
	}
	currThread->state=WAITRES;
	wn * waitNode=init_wn(currThread);
	waitNode->next=target->waiting;
	target->waiting=waitNode;
	my_pthread_yield();
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	if (!been_inited) {
		init();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
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
				__atomic_clear(&mode,0);
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
	if (!been_inited) {
		init();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	if(mutex==NULL||mutex->id==NULL){
		__atomic_clear(&mode,0);
		return -1;
	}

	if(mutex->currT==NULL){
		mutex->currT=currThread;
		currThread->state=LOCKING;
	}else{
		//printf("Gotta wait\n");
		wn * ptr=mutex->waiting;
		wn * waitNode=init_wn(currThread);
		currThread->state=WAITLOCK;
		currThread->mutex_id = mutex;
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
	if (!been_inited) {
		init();
		been_inited=1;
	}
	//printf("ulock status %d\n", currThread->state);
	__atomic_test_and_set(&mode,0);
	if(mutex==NULL||mutex->id==NULL||mutex->currT==NULL||mutex->currT->tid!=currThread->tid){
		//printf("Can't do that\n");
		//my_pthread_yield();
		__atomic_clear(&mode,0);
		return -1;
	}
	wn * ptr= mutex->waiting;
	currThread->mutex_id = NULL;
	currThread->wait_skips = 0;
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
//NEED TO TEST
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	if (!been_inited) {
		init();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	//printf("Destroying mutex: %d\n",*mutex);
	my_pthread_mutex_t* ptr = mutexPool->front;
	my_pthread_mutex_t * prev=NULL;
	if(mutex->currT!=NULL){
		my_pthread_yield();
		return -1;
	}

	while(mutex->waiting!=NULL){
		((tcb *)(((wn *)(mutex->waiting))->curr))->state=READY;
		((tcb *)(((wn *)(mutex->waiting))->curr))->mutex_id=NULL;
		((tcb *)(((wn *)(mutex->waiting))->curr))->wait_skips=0;
		mutex->waiting=((wn *)(mutex->waiting))->next;
	}
	/*while(mutex->waiting!=NULL){
	  ((tcb *)(((wn *)(mutex->waiting))->curr))->state=READY;
	  mutex->waiting=((wn *)(mutex->waiting))->next;
	  }*/
	while(ptr!= NULL) {
		if(ptr->id==mutex->id){
			if(prev!=NULL){
				prev->next=ptr->next;
			}else{
				mutexPool->front=NULL;
			}
			//free(mutex);

			mutex->id=NULL;

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
