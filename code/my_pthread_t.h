// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H
#define _GNU_SOURCE
#define pthread_create my_pthread_create
#define pthread_yield my_pthread_yield
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <stddef.h>
#include <malloc.h>
#include <signal.h>
#include <sys/mman.h>
#include <errno.h>
#ifndef MALLOC_HEADER
#define MALLOC_HEADER

//Using booleans
typedef int bool;
#define true 1
#define false 0



#define PAGE_SIZE 4096
#define MEM_SIZE 8388608
#define META_SIZE sizeof(meta) 		//size of the malloc_struc
#define malloc(x) myallocate(x, __FILE__, __LINE__, 1)
#define free(x) mydeallocate(x, __FILE__, __LINE__, 1)


typedef struct malloc_struc meta; 	//'struct meta_data_block' can be refered as 'block'

struct malloc_struc
{
    short isFree;
    meta *next;
    meta *prev;
    int size;
    char *data;
};

#define BLOCKSIZE sizeof(block) 		//size of the meta_data_block




//Paging Definitions

// Page table entry
typedef struct frame {
	//int isValid;
  	//bool hasMem;
	//char * bufferStart;
	//char * bufferEnd; //Should be an offset of 4 KB from bufferStart
	meta start;
	//struct page_entry * o_page;
} frame;


typedef struct page_entry{
	struct page_entry * prev;
	struct page_entry * next;	
	int index;
	int isValid;
	bool inRAM;
	long int swp_offset;	
	bool hasMem;
	struct frame * realFrame;
	struct frame * currFrame;
} page_entry;





//extern void handler (int sig, siginfo_t * si, void * unused);
extern page_entry * evict(int mode) ;
extern page_entry * getPage();
extern void set_pages();
extern page_entry * use_swap_page();
extern page_entry * use_page(int mode);
extern void swap_frames(page_entry * p1, page_entry * p2);
extern void * mallocBlock(size_t size, page_entry * currPage, int mode);
extern void freeBlock(void *ptr);

//Top-level function definitions
extern void * myallocate(size_t size, char * file, int line,  int mode);
extern void mydeallocate(void *ptr, char * file, int line, int mode);
#endif



//#include "OSProject2/my_malloc.h"


typedef uint my_pthread_t;

typedef struct wrapperstruct {
	void * args;
	void * func;
	struct threadControlBlock * tcb;
} ws;

typedef struct levelQueue{
	struct threadControlBlock * front;
} lq;

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	int id;
	struct my_pthread_mutex_t * next;
	struct threadControlBlock * currT;
	struct wn * waiting;
} my_pthread_mutex_t;

typedef struct wait_node{
	struct wait_node * next;
	struct threadControlBlock * curr;
} wn;

/* define your data structures here: */
typedef struct mutexP{
	struct my_pthread_mutex_t * front;
	int size;
}  mutexP;

typedef struct threadControlBlock {
	/* add something here */
	struct threadControlBlock * left;
	struct threadControlBlock * right;
	struct  wait_node * waiting;
	my_pthread_t * tid;
	ucontext_t * context;
	int state;
	struct my_pthread_mutex_t * mutex_id;
	int wait_skips;
	int priority;
	int num_drops;
	void ** res;
	struct wrapperstruct * args;
	void * func;

	page_entry ** addr_list;
	int page_count;

} tcb;

// Feel free to add your own auxiliary data structures

extern tcb * getCurrThread();
extern int getBeenInited();
extern void setBeenInited();
extern lq ** getScheduler();


/* Function Declarations: */

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
