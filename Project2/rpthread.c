// File:	rpthread.c

// List all group member's name: Alfonso Buono, Ali Mohamad
// username of iLab: ajb393, aam345
// iLab Server: kill

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

// linked list pointers.                        NEED TO FIX ENQUEUE AND DEQUEUE
tcb* runqueueH;
tcb* runqueueT;

// Global Schedule context                      NEED TO INITALIZE THIS AND SHIT
ucontext_t schedCon;

// print out linked list
void printList(){
    tcb* ptr = runqueueH;
    while(ptr != runqueueT){
        printf("%d->",ptr->threadID);
        ptr = ptr->next;
    }
    printf("%d\n",ptr->threadID);
}

void enqueue(tcb* node){
    return;
}

tcb* dequeue(){
    return NULL;
};

/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
    // Create Thread Control Block
    // Create and initialize the context of this thread
    // Allocate space of stack for this thread to run
    // after everything is all set, push this thread int
    // YOUR CODE HERE
	
    // Create TCB
    tcb* threadBlock = (tcb*) malloc(sizeof(tcb));
   
    // Create and initialize context
    ucontext_t context;
    getcontext(&context);    
    // make stack
    void *stack=malloc( SIGSTKSZ );
    // set data in context
    context.uc_link = NULL;
    context.uc_stack.ss_sp = stack;
    context.uc_stack.ss_size = SIGSTKSZ;
    context.uc_stack.ss_flags = 0;
    
    // include parameters ?
    makecontext( &context,(void(*)(void))function, 1,arg );
    
    // pthread_attr_init // dont have to do this
    

    // set data in tcb?
    threadBlock->threadID = 0; // ?
    threadBlock->threadStatus = ready; // ?
    threadBlock->context = context;
    threadBlock->stack = stack;
    threadBlock->priority = 0; //?
   
    // add the tcb into the runqueue
    //setcontext(&context);
    if(runqueueH == NULL){
        runqueueH = (tcb*) malloc( sizeof( tcb ) );
        runqueueT = (tcb*) malloc( sizeof( tcb ) );

        runqueueH->threadID = -1;
        runqueueH->next = runqueueT;

        runqueueT->threadID = -1;
        runqueueT->next = runqueueH;
        puts("first time calling pthread_create");
    }

    puts("pthread_create");
    if(runqueueT->next == runqueueH){
        runqueueT->next = threadBlock;
    }
    threadBlock->next = runqueueH->next;
    runqueueH->next = threadBlock;

    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	
	// Change thread state from Running to Ready
	// Save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE

    // change the information in the thread calling yield()
    currThread->status = ready;
    
    // swap context (saves curr context into the element that was just pushed into queue)
    swapcontext( &(currThread->context), &schedCont );
	
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
    
    // we need to deallocate the stack?
    // how do we get access to the correct thread / tcb to stop?
};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// Wait for a specific thread to terminate
	// De-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//Initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // When the mutex is acquired successfully, enter the critical section
        // If acquiring mutex fails, push current thread into block list and 
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// Every time when timer interrup happens, your thread library 
	// should be contexted switched from thread context to this 
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (STCF or MLFQ)

	// if (sched == STCF)
	//		sched_stcf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// schedule policy
#ifndef MLFQ
	// Choose STCF
#else 
	// Choose MLFQ
#endif

}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

