
// Author:	Yujie REN
// Date:	09/23/2017

// name: Hemanth Chiluka, Andrew Dos Reis, Benjamin Yang
// username of iLab: hkc33, ad1005, bty10
// iLab Server: adapter
#include "my_pthread_t.h"


//Mode constants
int LIBRARYREQ=0;
int THREADREQ=1;
int mode=0;
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
//int mode=0;
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
int stuckHereCtr = 0;
//ucontext_t * tailFunc;

tcb * currThread=NULL;


tcb * getCurrThread(){
	/*if(!been_inited){
		init();
	}*/
	return currThread;
}

lq ** getScheduler() {
	return scheduler;
}

int getBeenInited() {
	return been_inited;
}

void setBeenInited() {
	been_inited=1;
}


void signal_handler(){
	//printf("Caught signal\n");
	unprotect_sys();
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

	unprotect_sys();
	if(!isHandling){
		
		//printf("Scheduling\n");
		/*if(next_tcb->state==WAITRES){
		  printf("SHEET%d\n", next_tcb->state);
		  }*/
		//while(setitimer(ITIMER_VIRTUAL,&timer,NULL)==-1);

		if(cyclesLeft!=0){
			cyclesLeft--;
			protect_sys();
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
				unset_pages(currThread);
				ucontext_t * prevContext=currThread->context;
				currThread=next_tcb;
				isHandling=0;
				set_pages(currThread);
				//protect_sys();
				__atomic_clear(&mode,0);
				//printf("stuck here\n");
				stuckHereCtr++;
				//printf("stuck here counter is %d\n", stuckHereCtr);
				while(swapcontext(prevContext, next )<0);
			}else{
				isHandling=0;
				currThread=next_tcb;
				mprotect(&currThread,sizeof(tcb),PROT_READ|PROT_WRITE);
				set_pages(currThread);
				//protect_sys();
				__atomic_clear(&mode,0);
				//printf("stuck there\n");
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
		protect_sys();
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
	ucontext_t * tailFunc=myallocate(sizeof(ucontext_t),__FILE__,__LINE__, LIBRARYREQ);
	getcontext(tailFunc);
	tailFunc->uc_link=0;
	tailFunc->uc_stack.ss_sp=myallocate(memSize,__FILE__,__LINE__, LIBRARYREQ);
	tailFunc->uc_stack.ss_size=memSize;
	tailFunc->uc_stack.ss_flags=0;
	//if (getcontext(getCurrThread()->context) == -1) {
    	//	printf("error here\n");
	//}
	makecontext(tailFunc,(void *)&my_pthread_exit, 0);
	return tailFunc;
}

ucontext_t * init_context(void* func, void* arg){
	ucontext_t * t=(ucontext_t *) myallocate(sizeof(ucontext_t),__FILE__,__LINE__, LIBRARYREQ);
	while(getcontext(t)==-1);
	ucontext_t * tailFunc=init_tail();
	t->uc_link=tailFunc;
	t->uc_stack.ss_sp=myallocate(memSize,__FILE__,__LINE__, LIBRARYREQ);
	t->uc_stack.ss_size=memSize;
	t->uc_stack.ss_flags=0;
	//if (getcontext(getCurrThread()->context) == -1) {
    	//	printf("error here\n");
	//}
	makecontext(t,func, 1, arg);
	return t;
}


tcb * init_tcb(my_pthread_t * tid, ws * currArgs, void * func){
	tcb * curr=(tcb *)myallocate(sizeof(tcb),__FILE__,__LINE__, LIBRARYREQ);
	curr->right=NULL;
	curr->left=NULL;
	curr->tid=tid;
	curr->state=READY;
	curr->priority=0;
	curr->waiting=NULL;
	curr->res=myallocate(sizeof(void**),__FILE__,__LINE__, LIBRARYREQ);
	curr->args=currArgs;
	curr->func=func;
	curr->num_drops=0;
	curr->mutex_id=NULL;
	curr->wait_skips=0;
	
	curr->addr_list=(struct page_entry **)myallocate(1020*sizeof(page_entry *), __FILE__,__LINE__, LIBRARYREQ);
	if (curr->func != NULL) {
		(curr->args)->tcb=curr;
		curr->context=init_context(curr->func, curr->args);
	}
	else {
		ucontext_t * prevContext=myallocate(sizeof(ucontext_t),__FILE__,__LINE__, LIBRARYREQ);
		getcontext(prevContext);
		curr->context=prevContext;
	}
	return curr;
}

wn * init_wn(tcb * curr){
	wn * waitNode=(wn *)myallocate (sizeof(wn),__FILE__,__LINE__, LIBRARYREQ);
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
	
	scheduler=(lq **) myallocate(sizeof(lq *)*(numLevels+1),__FILE__,__LINE__,0);
	int i;
	for(i=0;i<numLevels+1;i++){
		(*(scheduler+i))=(lq *)myallocate(sizeof(lq),__FILE__,__LINE__, LIBRARYREQ);
		(*(scheduler+i))->front=NULL;
	}

	ucontext_t * prevContext=myallocate(sizeof(ucontext_t), __FILE__,__LINE__, LIBRARYREQ);
	my_pthread_t * prev_tid=(my_pthread_t *)myallocate(sizeof(my_pthread_t),__FILE__,__LINE__, LIBRARYREQ);
	*prev_tid=-1;
	getcontext(prevContext);
	prevContext->uc_link=init_tail();
	currThread=init_tcb(prev_tid, NULL, NULL);
	enqueue(currThread,currThread->priority);
	//Init tail context for every thread

	//init mutexpool
	mutexPool = (mutexP *) myallocate(sizeof(mutexP),__FILE__,__LINE__, LIBRARYREQ);

	//init alarm
	while(signal(SIGVTALRM,(void *)&checkSigHandler)==SIG_ERR);
	timer.it_value.tv_sec=0;
	timer.it_value.tv_usec=250000;
	timer.it_interval=timer.it_value;
	while(setitimer(ITIMER_VIRTUAL,&timer,NULL)==-1);

	//sigprocmask( SIGVTALRM,&set,NULL);
	/*sigemptyset(&emptySet);
	sigemptyset(&blockSet);
	sigaddset(&blockSet, SIGVTALRM);
	*/
	been_inited=1;
	//printf("Done initializing\n");
	//while(1==1);
}



/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	__atomic_test_and_set(&mode,0);
	if (!been_inited) {
		init_memory();
		been_inited=1;
	}
	unprotect_sys();
	

	//ucontext_t * my_context = init_context(function,  arg);
	//setcontext(&my_context);
	ws * currArgs = (ws *) myallocate(sizeof(ws),__FILE__,__LINE__, LIBRARYREQ);
	currArgs->func = function;
	currArgs->args = arg;
	*thread=threadCount;
	threadCount++;
	tcb * curr= init_tcb(thread, currArgs, &wrapper);

	enqueue(curr, 0);
	protect_sys();
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
	if (!been_inited) {
		init_memory();
		been_inited=1;
	}
	unprotect_sys();
	__atomic_test_and_set(&mode,0);
	
	cyclesLeft=0;
	signal_handler();
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	if (!been_inited) {
		init_memory();
		been_inited=1;
	}
	unprotect_sys();
	
	__atomic_test_and_set(&mode,0);
	//printf("Exiting\n");


	currThread->state=TERMINATED;
	removeThread(currThread);
	free_pages(currThread);
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
		init_memory();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	unprotect_sys();
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
		init_memory();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	unprotect_sys();
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
				protect_sys();
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
		init_memory();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	unprotect_sys();
	if(mutex==NULL||mutex->id==NULL){
		protect_sys();
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
		init_memory();
		been_inited=1;
	}
	//printf("ulock status %d\n", currThread->state);
	__atomic_test_and_set(&mode,0);
	unprotect_sys();
	if(mutex==NULL||mutex->id==NULL||mutex->currT==NULL||mutex->currT->tid!=currThread->tid){
		//printf("Can't do that\n");
		//my_pthread_yield();
		protect_sys();
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
		init_memory();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	unprotect_sys();
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





//MALLOC CODE STARTS HERE



static short start = 0;
static char * myMemory; //character array which is used as memory
page_entry * page_table;
bool isSys=0;

//const int SYS_MEM_END=MEM_SIZE/8; //Point in char array where sys memory ends
//const int SHARED_MEM_SIZE=4*PAGE_SIZE; //Shared memory size
//const int PAGE_SIZE=4096;
//
const int TOTAL_MEM=8388608;
const int PAGE_TABLE_SIZE=120;
const int PAGE_TABLE_MEM=491520;
const int NUM_PAGES=2048+4096;
const int MEM_PAGES=2048;
const int OS_PAGE_START = 200;
const int USER_PAGE_START = 1024;
const int SHARED_PAGE_START=2044;

char * OS_HARD_END;
char * USER_HARD_END;
int OS_PAGE_COUNT = 0;
int USER_PAGE_COUNT = 0;
int SHARED_PAGE_COUNT=1;
FILE * writer;

int EVICTMODE = 1;

int currOff=0;
int curr_swp_bytes=0;
bool RAM_FULL=false;
page_entry * freeHead=NULL;

void testMem(char * ptr){
	*ptr='a';
}

//init functions
static void handler(int sig, siginfo_t *si, void *unused) {
	printf("Got SIGSEGV at address: 0x%lx\n",(long) si->si_addr);
	tcb * currThread = getCurrThread();
	printf("currthread is: %d\n", currThread->tid);
	long prob_addr = (long) si->si_addr;
	page_entry ** addrs = currThread->addr_list;
	unprotect_sys();
	int i;
	for (i = 0; i < currThread->page_count; i++) {
		page_entry * currPage = addrs[i];
		if ((currPage->realFrame) <= prob_addr && ((char *)(currPage->realFrame)+PAGE_SIZE>prob_addr)){
			printf("valid page: %d\n", prob_addr);

			if(currPage->inRAM){
				if (currPage->in_mem_swapped) {
					page_entry * imposter = getPage(currPage->realFrame);
					swap_frames(currPage, imposter);
					currPage->in_mem_swapped=false;
				}
				else {
					page_entry * imposter=getPage(currPage->realFrame);
					swap_frames(currPage,imposter);
				}
			}else{
				page_entry * victim=evict(EVICT_MODE);
				to_swap_file(victim, currPage,false);
			}
			protect_sys();
			return;
		}
	}

	signal(SIGSEGV, SIG_DFL);
}

void protect_sys(){
	//if(!isSys) return;
	/*int res=mprotect(myMemory,(USER_PAGE_START)*PAGE_SIZE,PROT_NONE); //Protect all un malloced pages except one with page table
	  if(res==-1) printf("damn\n");*/
	isSys=false;
}

void unprotect_sys(){
	//if(isSys) return ;
	int res=mprotect(myMemory,(USER_PAGE_START)*PAGE_SIZE,PROT_READ|PROT_WRITE); //Protect all un malloced pages except one with page table

	if(res==-1) printf("damn\n");
	isSys=true;
}

page_entry * getPage(frame * frame){
	mprotect(frame,PAGE_SIZE,PROT_READ|PROT_WRITE);
	int i;
	for (i = OS_PAGE_START; i < NUM_PAGES; i++) {
		if (page_table[i].currFrame == frame) {
			//mprotect(frame,PAGE_SIZE,PROT_NONE);
			return &(page_table[i]);
		}
	}
	return NULL;
}

frame * getFrame(int index){
	frame * f = (frame *) (myMemory + (index * PAGE_SIZE));
	return f;
}

frame * getFrameFromPtr(char * ptr){
	int i = (((int) ptr-1) - ((int) myMemory))/PAGE_SIZE;
	return myMemory+PAGE_SIZE*i;
}

meta * findLastMeta(frame * frame){
	meta * start= &(frame->start);
	while(start->next!=NULL){
		start=start->next;

	}
	return start;
}
//release pages when thread dies
void free_pages(tcb * thread){
	int i;
	for(i=0;i<thread->page_count;i++){
		page_entry * page = thread->addr_list[i];
		page->isValid=0;
		memset(page->currFrame,0,PAGE_SIZE);
		frame * frame=page->currFrame;
		meta * start= &(frame->start);
		start->size=PAGE_SIZE-sizeof(meta);
		start->prev=NULL;
		start->next=NULL;
		start->data=(char *)start+ sizeof(meta);
		start->isFree=1;
	}
	unset_pages(thread);

}

deleteFree(page_entry * page){
	
	page_entry * left= page->prev;
	page_entry * right=page->next;
	if(left!=NULL){
		left->next=right;
	}

	if(right!=NULL){
		right->prev=left;
	}
	if(page==freeHead){
		freeHead=freeHead->next;
	}
	page->prev=NULL;
	page->next=NULL;

}

addFree(page_entry * ptr){
	if(freeHead==NULL){
		freeHead=ptr;
		ptr->next=NULL;
		ptr->prev=NULL;
		return;
	}
	ptr->next=freeHead;
	freeHead->prev=ptr;
	freeHead=ptr;


}
//protects previous thread's pages
void unset_pages(tcb * thread){
	int i;
	for (i=0;i<thread->page_count;i++){
		page_entry * page=thread->addr_list[i];
		if(page->inRAM){
			addFree(page);
		}
		mprotect(page->currFrame, PAGE_SIZE,PROT_NONE);
	}

}

//unprotects curr thread's pages
void set_pages(tcb * curr){

	int i;
	for (i=0;i<curr->page_count;i++){
		page_entry * page=curr->addr_list[i];
		deleteFree(page);
		/*if(page->realFrame!=page->currFrame){
		  page_entry * imposter= getPage(page->realFrame);
		  swap_frames(page,imposter);
		}*/
		  mprotect(page->currFrame,PAGE_SIZE,PROT_READ|PROT_WRITE);
	}

}




//swaps frames of two pages, mprotects for the new swapped in frame
//Mmay make sense to set p1's real frame here
void swap_frames(page_entry * p1, page_entry * p2) {
	mprotect(p1->currFrame,PAGE_SIZE,PROT_READ|PROT_WRITE);
	mprotect(p2->currFrame,PAGE_SIZE,PROT_READ|PROT_WRITE);

	frame * f1 = p1->currFrame;
	frame * f2 = p2->currFrame;
	char temp[PAGE_SIZE];
	
	if(f1==NULL){
	
	}
	memcpy(temp, f1, PAGE_SIZE);
	memcpy(f1, f2, PAGE_SIZE);
	memcpy(f2, temp, PAGE_SIZE);

	p1->currFrame=f2;
	p2->currFrame=f1;

	mprotect(p2->currFrame, PAGE_SIZE, PROT_NONE);
	mprotect(p1->currFrame,PAGE_SIZE, PROT_READ|PROT_WRITE);
}

void mergeFrames(meta * firstEnd, frame * second){
	char * end= (char *) firstEnd+ sizeof(meta)+firstEnd->size;
	meta * secondStart=&(second->start);
	firstEnd->next=secondStart;
	secondStart->prev=firstEnd;
	if(secondStart->next!=NULL){
		secondStart->next=NULL;
	}	
	if(firstEnd->isFree&&secondStart->isFree){
		firstEnd->next=secondStart->next;
		firstEnd->size=firstEnd->size+sizeof(meta)+secondStart->size;
	}else{
	//	printf("Why not free?\n");
	}

}


void to_swap_file(page_entry * to_swap, page_entry * to_RAM, bool isNew){
	//FILE * orig=writer;
	to_swap->inRAM=false;
	to_swap->swp_offset=to_RAM->swp_offset;
	curr_swp_bytes+=PAGE_SIZE;
	to_RAM->inRAM=true;
	to_RAM->swp_offset=-1;
	to_RAM->currFrame=to_swap->currFrame;

	mprotect(to_RAM->currFrame, PAGE_SIZE, PROT_READ|PROT_WRITE);
	int error=fseek(writer,(to_swap->swp_offset)*PAGE_SIZE,SEEK_SET);	
	if(error==0){
		char buffer[PAGE_SIZE];
		currOff=to_swap->swp_offset;
		fread(buffer, 1,4096,writer);
		fseek(writer, (to_swap->swp_offset)*PAGE_SIZE,SEEK_SET);

		char temp[PAGE_SIZE];
		memcpy(temp, to_swap->currFrame, 4096);
		fwrite(temp,1, 4096, writer);

		memcpy( to_RAM->currFrame,buffer, 4096);
		
		fseek(writer, (to_swap->swp_offset)*PAGE_SIZE, SEEK_SET);
		char test[PAGE_SIZE];
		fread(test, 1,4096,writer);
		
		meta * start= &(to_RAM->currFrame->start);
		if(isNew){
			start->isFree=true;
			start->size=PAGE_SIZE-sizeof(meta);
			start->prev=NULL;		
			start->next=NULL;
			start->data=(char *)start+sizeof(meta);	
		}else{
			int x=4;
		}
		to_swap->currFrame=NULL;
	}
	//fclose(orig);

}

page_entry * evict(int mode){
	tcb * thread=getCurrThread();
	if(mode==0){
		int i;

		for (i=USER_PAGE_START;i<NUM_PAGES;i++){
			if(i>=SHARED_PAGE_START&&i<MEM_PAGES){
				continue;
				
			}
			page_entry * p =&page_table[i];
			int j;
			bool isMine=false;
			for(j=0;j<thread->page_count;j++){
				if(thread->addr_list[j]==p){
					isMine=true;
					break;
				}
			}
			if(!isMine&&(p->inRAM==1)&&p->currFrame!=NULL){
				return p;
			}

		}

	}
	if (mode==1) {//second chance
		int i;
		page_entry * ptr;
		for (i = 0; i < 2; i++) {//change < 2 for nth chance
			ptr = freeHead;
			while (ptr->next != NULL) {
				if (ptr->use_bit == 0) {//found victim; in memory swapping here;
					page_entry * check_high_used = freeHead;
					page_entry * highest_used = ptr;
					bool ptr_is_highest_used = true;
					while (check_high_used->next != NULL) {
						if (check_high_used->mem_use_bit > highest_used->mem_use_bit) {
							ptr_is_highest_used=false;
							break;
						}
					}
					if (ptr_is_highest_used) {
						page_entry * new_victim;
						if (ptr == freeHead) {
							new_victim = ptr->next;
						}
						else {
							new_victim = freeHead;
						}
						//move highest_used's frame to new_victim's frame
						//move new_victim's frame to swap file (in handler)
						//move currPage's frame to highest_used's frame (in handler)
						swap_frames(new_victim, ptr);
						ptr->in_mem_swapped=true;
						return new_victim;
					}
					deleteFree(ptr);
					return ptr;
				}
				else if (ptr->use_bit > 0) {//change to > 0 if nth chance
					ptr->use_bit = 0;
					ptr = ptr->next;
				}
			}
		}
		if (ptr != NULL) {
			//checking if highest used
			
			ptr->use_bit = 0;
			deleteFree(ptr);
			return ptr;
		}
	}
	return NULL;
}

void inmem_swap_frames(page_entry * p1, page_entry * p2) {
	mprotect(p1->currFrame,PAGE_SIZE,PROT_READ|PROT_WRITE);
	mprotect(p2->currFrame,PAGE_SIZE,PROT_READ|PROT_WRITE);

	frame * f1 = p1->currFrame;
	frame * f2 = p2->currFrame;
	char temp[PAGE_SIZE];

	memcpy(temp, f1, PAGE_SIZE);
	memcpy(f1, f2, PAGE_SIZE);
	memcpy(f2, temp, PAGE_SIZE);

	p1->currFrame=f2;
	p2->currFrame=f1;

	mprotect(p2->currFrame, PAGE_SIZE, PROT_NONE);
	mprotect(p1->currFrame,PAGE_SIZE, PROT_READ|PROT_WRITE);
}


page_entry * add_user_page(tcb * thread){

	if(thread->page_count>=1020){
		return NULL; //Frame is out of bounds 
	}
	page_entry * page= use_page(THREADREQ);
	if(page==NULL){
		page=use_swap_page();
		if(page==NULL){
			return NULL;
		}
	
		page_entry * victim = evict(EVICT_MODE);
		if(victim==NULL){
			printf("No victim\n");
			return NULL;
		}
		
		mprotect(victim->currFrame,PAGE_SIZE, PROT_WRITE|PROT_READ);
		to_swap_file(victim,page,true);
	}	

	if(page->currFrame==NULL){
		printf("PAGE CURR FRAME NULL WHAAAAT\n");
	}
	if(thread->page_count>0){
		page_entry * prev=thread->addr_list[thread->page_count-1];
		frame * target= (frame *)((char *)prev->realFrame+PAGE_SIZE);
		page_entry * targetPage=getPage(target);
		swap_frames(page,targetPage);

		(page->realFrame)=page->currFrame;
		mprotect(page->currFrame,PAGE_SIZE, PROT_READ|PROT_WRITE); //Page now available to thread
	}else{
		frame * start = myMemory+ (PAGE_SIZE *USER_PAGE_START);
		page_entry * startPage= getPage(start);
		
		swap_frames(page,startPage);
		(page->realFrame)=page->currFrame;
		mprotect(page->currFrame,PAGE_SIZE, PROT_READ|PROT_WRITE); //Page now available to thread
	}

	page->currFrame->start.data=&(page->currFrame->start)+1;

	page->isValid=1;
	thread->addr_list[thread->page_count]=page;
	thread->page_count++;

	return page;

}

page_entry * add_shared_page(){
	if(SHARED_PAGE_COUNT>=4) return NULL;
	page_entry * p=&page_table[SHARED_PAGE_START+SHARED_PAGE_COUNT];	
	SHARED_PAGE_COUNT++;
	return p;
}
page_entry * add_sys_page(){
	page_entry * page=use_page(LIBRARYREQ);

	return page;
}


frame * init_frame(page_entry * pe, int offset) {
	frame * f = (frame *) (myMemory + (offset * PAGE_SIZE));
	//f->isValid = 0;
	//f->o_page=pe;
	//f->start = (char *)f+sizeof(frame);
	f->start.isFree=1;
	f->start.next=NULL;
	f->start.prev=NULL;
	f->start.size=PAGE_SIZE-sizeof(meta);
	f->start.data=&(f->start)+1;
	//f->hasMem = false;
	return f;
}


//init page table
void init_memory() {
	been_inited=1;
	__atomic_test_and_set(&mode,0);
	writer = fopen("project2.swp", "w+");
	myMemory=(char *)memalign(PAGE_SIZE, TOTAL_MEM);
	init_page_table();
	/*page_entry * pt= (page_entry*) (myMemory);
	  pt->index = OS_PAGE_COUNT;
	  pt->realFrame = init_frame(pt,1);
	  pt->currFrame=pt->realFrame;*/
	OS_HARD_END = (char *) myMemory + PAGE_SIZE*1024;
	USER_HARD_END = (char *) myMemory + PAGE_SIZE*2048;
	OS_PAGE_COUNT+=124;
	init_signal();	
	init();
	protect_sys();
	mprotect(myMemory+PAGE_SIZE*1024,PAGE_SIZE* 1020,PROT_NONE);
	__atomic_clear(&mode,0);
}

void init_signal() {
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = handler;

	if (sigaction(SIGSEGV, &sa, NULL) == -1)
	{
		printf("Fatal error setting up signal handler\n");
		exit(EXIT_FAILURE);    //explode!
	}
}

page_entry * use_swap_page(){
	int i;
	tcb * curr= getCurrThread();
	for(i=USER_PAGE_START;i<NUM_PAGES;i++){
		page_entry * page= &(page_table[i]);
		int j;
		bool isMine=false;
		for (j=0;j<curr->page_count;j++){
			if(curr->addr_list[j]==page){
				isMine=true;
				break;
			}
		}
		if((!page->inRAM)&&(!isMine)&&(!page->isValid)){
			return page;
		}
	} 
	return NULL;

}

void check_sys_list(){
	
	meta * ptr= &(page_table[OS_PAGE_START].currFrame->start);
	int i=0;
	while(ptr!=NULL){
		if(ptr->isFree>1){
			printf("Ptr fuked at %d\n",i);
		}
		i++;
		ptr=ptr->next;
	}
}

page_entry * use_page(int mode) {
	//tcb * curr=getCurrThread();
	check_sys_list();
	if (mode==LIBRARYREQ) {
		if(OS_PAGE_START+OS_PAGE_COUNT<1024){
			page_entry * p = &page_table[OS_PAGE_START+OS_PAGE_COUNT];
			p->index = OS_PAGE_COUNT+OS_PAGE_START;
			p->currFrame=myMemory+(p->index*PAGE_SIZE);
			p->realFrame=myMemory+(p->index*PAGE_SIZE);
			OS_PAGE_COUNT++;
			return p;

		}else{
			return NULL;
		}
		
	}
	else if (mode ==THREADREQ){
		/*page_entry * p = &page_table[USER_PAGE_START + USER_PAGE_COUNT];
		  p->index = USER_PAGE_START + USER_PAGE_COUNT;
		  USER_PAGE_COUNT++;*/
		
		int i;
		for(i=USER_PAGE_START;i<MEM_PAGES;i++){
			if(i>=SHARED_PAGE_START&&i<MEM_PAGES){
				continue;
			}
			page_entry * ptr = &page_table[i];
			if(ptr->inRAM==1&&ptr->isValid==0){
				//deleteFree(ptr);
				return ptr;

			}
		}
		
		return NULL;
	}
	check_sys_list();
	printf("Wrong\n");
	return NULL;
}


void init_page_table(){
	page_table=(void *)myMemory;
	int i;

	for(i=NUM_PAGES-1;i>=OS_PAGE_START;i--){
		page_table[i].index=i;
		page_table[i].isValid=0;
		page_table[i].hasMem=false;
		if(i<MEM_PAGES){
			page_table[i].inRAM=true;
			frame * currFrame=init_frame(&page_table[i],i);
			page_table[i].realFrame=currFrame;
			page_table[i].currFrame=currFrame;
		}else{
			page_table[i].swp_offset=i-MEM_PAGES;
		}
		if(i>1024&&i<(SHARED_PAGE_START)){
			if(freeHead==NULL){
				freeHead=&(page_table[i]);
			}else{
				page_entry * ptr = &page_table[i];
				ptr->next=freeHead;
				freeHead->prev=ptr;
				freeHead=ptr;
			}
		}
	}
	int y=6;
}




void * mallocSharedBlock(size_t size){
	page_entry * page=&page_table[SHARED_PAGE_START];
	meta * ptr = &((page->currFrame)->start);

	while((ptr)!= NULL) {//this loop searches for a meta that doesn't point to another node

		if((*ptr).isFree == 1 && (*ptr).size >= size) {	//again checks for a reusable meta of same size

			(*ptr).isFree = 0;
			errno = 0;
			int rem =ptr->size-(sizeof(meta)+size);
			if(rem>0){
				meta * remPtr= (char *)(ptr+1)+size;
				remPtr->isFree=1;
				remPtr->data=remPtr+1;
				remPtr->size=rem;
				remPtr->next=ptr->next;
				ptr->size=size;
				ptr->next = remPtr;
				if(remPtr->next!=NULL){
					remPtr->next->prev=remPtr;
				}
				remPtr->prev=ptr;
				if(remPtr->prev!=NULL){
					remPtr->prev->next=remPtr;
				}
			}

			//printf("ok, ");
			return (*ptr).data;

		}

		if(ptr->next==NULL){
			page_entry * extra = add_shared_page();
			if(extra==NULL){ //couln't get another pa-ge
				break;
			}
			mprotect(extra->currFrame,PAGE_SIZE,PROT_READ|PROT_WRITE);
			mergeFrames(ptr,extra->currFrame);
			continue;	
		}
		ptr = (*ptr).next;

	}

	return NULL;


}

void * mallocThreadBlock(size_t size, tcb * thread){
	check_sys_list();
	if(thread->page_count==0){
		add_user_page(thread);	
	}
	check_sys_list();

	page_entry * currPage=thread->addr_list[0];
	meta * ptr = &((currPage->currFrame)->start);

	while((ptr)!= NULL) {//this loop searches for a meta that doesn't point to another node

		if((*ptr).isFree == 1 && (*ptr).size >= size) {	//again checks for a reusable meta of same size
			check_sys_list();
			(*ptr).isFree = 0;
			errno = 0;
			int rem =ptr->size-(sizeof(meta)+size);
			if(rem>0){
				meta * remPtr= (char *)(ptr+1)+size;
				remPtr->isFree=1;
				remPtr->data=remPtr+1;
				remPtr->size=rem;
				remPtr->next=ptr->next;
				ptr->size=size;
				ptr->next = remPtr;
				if(remPtr->next!=NULL){
					remPtr->next->prev=remPtr;
				}
				remPtr->prev=ptr;
				if(remPtr->prev!=NULL){
					remPtr->prev->next=remPtr;
				}
			}
			check_sys_list();
			//printf("ok, ");
			return (*ptr).data;

		}

		if(ptr->next==NULL){
			check_sys_list();
			page_entry * extra = add_user_page(thread);
			if(extra==NULL){ //couln't get another pa-ge
				break;
			}
			mprotect(extra->currFrame,PAGE_SIZE,PROT_READ|PROT_WRITE);
			mergeFrames(ptr,extra->currFrame);
			check_sys_list();
			continue;	
		}
		ptr = (*ptr).next;

	}

	return NULL;


}

void * mallocSysBlock(size_t size){

	meta * ptr= &(page_table[OS_PAGE_START].currFrame->start);
	while((ptr)!= NULL) {//this loop searches for a meta that doesn't point to another node
		if((*ptr).isFree == 1 && (*ptr).size >= size) {	//again checks for a reusable meta of same size
			check_sys_list();
			(*ptr).isFree = 0;
			errno = 0;
			int rem =ptr->size-(sizeof(meta)+size);
			if(rem>0){
				meta * remPtr= (char *)(ptr+1)+size;
				remPtr->isFree=1;
				remPtr->data=remPtr+1;
				remPtr->size=rem;
				remPtr->next=ptr->next;
				ptr->size=size;
				ptr->next = remPtr;
				if(remPtr->next!=NULL){
					remPtr->next->prev=remPtr;
				}
				remPtr->prev=ptr;
				if(remPtr->prev!=NULL){
					remPtr->prev->next=remPtr;
				}
			}
			check_sys_list();
			//printf("ok, ");
			return (*ptr).data;

		}

		if(ptr->next==NULL){
			check_sys_list();
			page_entry * extra = add_sys_page();
			if(extra==NULL){ //couln't get another pa-ge
				break;
			}
			extra->realFrame=extra->currFrame;
			mergeFrames(ptr,extra->realFrame);
			check_sys_list();
			continue;	
		}
		ptr = (*ptr).next;

	}


}

void freeBlock(void *pointer){
	check_sys_list();
	meta* ptr = (meta*)pointer - 1; //gets the meta * ptr that points to the malloced pointer


	//printf("freed, ");
	(*ptr).isFree = 1;		//inUse is set to 0 because the pointer was freed

	meta * mergePtr = (*ptr).next;	//if the next meta is not in use the metas must be fused
	if(mergePtr != NULL) {	//the meta left is the one currently pointed to ie ptr

		if((*mergePtr).isFree == 1) {

			(*ptr).size = (*ptr).size + (*mergePtr).size + sizeof(meta);
			(*ptr).next = (*mergePtr).next;
			if((*mergePtr).next != NULL) {
				mergePtr->next->prev = ptr;
			}

		}

	}

	mergePtr = (*ptr).prev;		//if the previous meta is not in use the metas must be merged
	if(mergePtr != NULL) {		//meta left is the  previous to the pointer that we freed

		if((*mergePtr).isFree == 1) {

			(*mergePtr).size = (*mergePtr).size + (*ptr).size + sizeof(meta);
			(*mergePtr).next = (*ptr).next;
			if((*ptr).next != NULL) {

				ptr->next->prev = mergePtr;
			}

		}

	}
	check_sys_list();
	errno = 0;		//if free succeeds then errno is set to zero otherwise it is set to -1


}

void * myallocate(size_t size,char * file, int line,  int sys) {
	if (!been_inited) {
		init_memory();
		been_inited=1;
	}
	int i;
	void * ptr= NULL;
	__atomic_test_and_set(&mode,0);
	if (sys == LIBRARYREQ) {
		ptr=mallocSysBlock(size);
	}
	else if (sys == THREADREQ) {
		unprotect_sys();
		tcb * currThread= getCurrThread();
		page_entry ** list = currThread->addr_list;
		ptr=mallocThreadBlock(size, currThread);
		protect_sys();

	}
	__atomic_clear(&mode,0);
	return ptr;
}

void * shalloc(size_t size){
	if (!been_inited) {
		init_memory();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	unprotect_sys();
	void * ptr=mallocSharedBlock(size);	
	protect_sys();
	__atomic_clear(&mode,0);
	return ptr;

}

void mydeallocate(void *ptr, char * file, int line, int mode){
	if (!been_inited) {
		init_memory();
		been_inited=1;
	}
	__atomic_test_and_set(&mode,0);
	unprotect_sys();
	freeBlock(ptr );
	protect_sys();
	__atomic_clear(&mode,0);
	//freeBlock(ptr, NULL);
}
