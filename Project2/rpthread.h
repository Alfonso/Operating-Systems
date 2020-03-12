// File:	rpthread_t.h

// List all group member's name: Alfonso Buono, Ali Mohamad
// username of iLab: ajb393, aam345
// iLab Server: kill

#ifndef RTHREAD_T_H
#define RTHREAD_T_H

#define _GNU_SOURCE

#define INTERVAL 5


/* To use Linux pthread Library in Benchmark, you have to comment the USE_RTHREAD macro */
#define USE_RTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

typedef uint rpthread_t;

typedef enum status{ running, ready, blocked, terminated } status;

typedef struct threadControlBlock {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
	rpthread_t threadID;
    status threadStatus;
    ucontext_t context;
    void* stack;
    int priority;
    struct threadControlBlock* next;
    struct threadControlBlock* prev;
    struct threadControlBlock* Mnext;
    struct threadControlBlock* Mprev;
    void* retval;
} tcb; 

/* mutex struct definition */
typedef struct rpthread_mutex_t {
	/* add something here */

	// YOUR CODE HERE
    tcb* owner;
    tcb* queueH;
    tcb* queueT;
    volatile int isLocked;
    

} rpthread_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE


/* Function Declarations: */

/*      Private functions created       */
static void sched_rr();
void sched_context_create();
void printList();
void printBlock();
void enqueue(tcb*,int);
tcb* dequeue(int);
tcb* findTCB(rpthread_t, int);
int isEmpty();
void sig_handler(int);


/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int rpthread_yield();

/* terminate a thread */
void rpthread_exit(void *value_ptr);

/* wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr);

/* initial the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex);

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex);

/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex);

static void schedule();

static void sched_stcf();

static void sched_mlfq();

#ifdef USE_RTHREAD
#define pthread_t rpthread_t
#define pthread_mutex_t rpthread_mutex_t
#define pthread_create rpthread_create
#define pthread_exit rpthread_exit
#define pthread_join rpthread_join
#define pthread_mutex_init rpthread_mutex_init
#define pthread_mutex_lock rpthread_mutex_lock
#define pthread_mutex_unlock rpthread_mutex_unlock
#define pthread_mutex_destroy rpthread_mutex_destroy
#endif

#endif
