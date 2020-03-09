// File:	rpthread.c

// List all group member's name: Alfonso Buono, Ali Mohamad
// username of iLab: ajb393, aam345
// iLab Server: kill

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

// linked list pointers.                    
tcb* runqueueH;
tcb* runqueueT;
tcb* terminatedH;

// Global current thread counter
tcb* currThread;

// Global Schedule context                      NEED TO INITALIZE THIS AND SHIT
ucontext_t schedCont;
int initialSched = 0;

// thread id counter
rpthread_t threadCounter = 2;

// global timer
struct itimerval it_val;

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
    getcontext(&(threadBlock->context));
    // make stack
    void *stack=malloc( SIGSTKSZ );
    // set data in context
    threadBlock->context.uc_link = NULL;
    threadBlock->context.uc_stack.ss_sp = stack;
    threadBlock->context.uc_stack.ss_size = SIGSTKSZ;
    threadBlock->context.uc_stack.ss_flags = 0;
    
    // include parameters ?
    makecontext( &(threadBlock->context),(void(*)(void))function, 1,arg );
    
    // pthread_attr_init // dont have to do this

    // set data in tcb?
    threadBlock->threadID = threadCounter; // ?
    threadCounter += 1;
    threadBlock->threadStatus = ready; // ?
    threadBlock->stack = stack;
    threadBlock->priority = 0; //?
  
    *thread = threadBlock->threadID;
    // add the tcb into the runqueue
    enqueue( threadBlock, 0 );


    // make scheduler context
    if( initialSched == 0 ){
        sched_context_create();
    }


    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	
	// Change thread state from Running to Ready
	// Save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE

    // turn the timer off
    struct itimerval zero;
    zero.it_value.tv_sec = 0;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_PROF,&zero,NULL);
    printf("yield(): Thread: %u is giving up control\n",currThread->threadID);
    
    // change the information in the thread calling yield()
    currThread->threadStatus = ready;
    
    // swap context (saves curr context into the element that was just pushed into queue)
    swapcontext( &(currThread->context), &schedCont );
	
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
    
    // we need to deallocate the stack?
    // do we have a currThread ptr to deallocate it?
    //free( currThread->stack );
    
    // set the thread status to terminate?
    currThread->threadStatus = terminated;

    printf("exit(): Thread exiting: %u, status: %d\n",currThread->threadID,currThread->threadStatus);

    // Something with the value_ptr?                        What is this

 
    // turn the timer off
    struct itimerval zero;
    zero.it_value.tv_sec = 0;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_PROF,&zero,NULL);
    puts("exit(): turned timer off going back to sched");


    // return back to the scheduler?                        Do I need to do this?
    setcontext( &schedCont );

    return;
};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// Wait for a specific thread to terminate
	// De-allocate any dynamic memory created by the joining thread
 
    // find the tcb associated to thread
    tcb* temp = findTCB( thread, -1 );
    printf("join(): searching for: %u\n",thread);
    // infinitely loop to block thread that called this until thread passed into parameters is terminated
    while( temp->threadStatus != terminated ){
    }
    puts("join(): In join after while");

/*
    // deallocate memory
    if( temp->stack )
        free( temp->stack );
*/
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

    // set the signal handler
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &sig_handler;
    sigaction (SIGPROF, &sa, NULL);    

    it_val.it_value.tv_sec = INTERVAL/1000;
    it_val.it_value.tv_usec = (INTERVAL*1000) % 1000000;
    

    while( isEmpty() != 1 ){
        // check which scheduling algo we are using
        // schedule policy
        /*
        #ifndef MLFQ
	        // Choose STCF
	        //sched = STCF;
	        // testing with FCFS instead of using STCF
        #else
	        // Choose MLFQ
	        //sched = MLFQ;
            sched_mlfq();
        #endif
        */
        printf("sched(): before: ");
        printList();
        sched_rr();
        currThread->threadStatus = running;
        printf("    Switching threads to: %u   ",currThread->threadID);
        printf("After: ");
        printList();
        puts("");

        // start the timer
        setitimer(ITIMER_PROF,&it_val,NULL);
        
        // swap context
        swapcontext( &schedCont, &(currThread->context) );
    
        // do we put the thread back into the runqueue? have to check if its terminated or not?
        if(currThread->threadStatus != terminated){
            currThread->threadStatus = ready;
            printf("sched(): Adding %u to Runqueue\n", currThread->threadID);
            enqueue( currThread, 0 );
        }else if(currThread->threadStatus == terminated){
            printf("sched(): Adding %u to terminated\n", currThread->threadID);
            enqueue( currThread, -1 );
        }

    }
}

/* non-preemptive FCFS scheduling algorithm */
static void sched_rr(){
    // Used to test our threading functions
    currThread = dequeue(0);
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


// sets the global schedule context
// Called once
void sched_context_create(){
    initialSched = 1;
    // This is making the global schedule context
    getcontext(&schedCont);
    void *schedStack=malloc( SIGSTKSZ);
    schedCont.uc_link = NULL;
    schedCont.uc_stack.ss_sp = schedStack;
    schedCont.uc_stack.ss_size = SIGSTKSZ;
    schedCont.uc_stack.ss_flags = 0;
    makecontext( &schedCont,(void(*)(void))schedule,0,NULL);

    // keep track of the main context and putit in the linked list
    // create a tcb and store it in their?
    tcb* mainTCB = (tcb*) malloc( sizeof( tcb ) );
    mainTCB->threadID = 1;
    // we dont have access to the stack of main                 Is this okay?
    mainTCB->stack = NULL;
    // Have to set the prioirty of this. Do we set it as high as possible?  IS this okay?
    mainTCB->priority = 0;
    mainTCB->threadStatus = ready;
    // enqueue it into the runqueue
    enqueue( mainTCB, 0 );
    
    getcontext( &(mainTCB->context) );
    
    swapcontext(&(mainTCB->context), &schedCont);
}

// print out linked list
void printList(){
    tcb* ptr = runqueueH;
    while(ptr != runqueueT){
        printf("%u->",ptr->threadID);
        ptr = ptr->next;
    }
    printf("%u",ptr->threadID);
}

void enqueue(tcb* node,int queue){
    //check which queue Im adding it to:
    if( queue == 0){
        // traditional runqueue
        // check if this is the first time // head and tail getting initialized
        if(runqueueH == NULL){
            runqueueH = (tcb*) malloc( sizeof( tcb ) );
            runqueueT = (tcb*) malloc( sizeof( tcb ) );

            runqueueH->threadID = 0;
            runqueueH->next = runqueueT;
            runqueueH->prev = NULL;

            runqueueT->threadID = 0;
            runqueueT->prev = runqueueH;
            runqueueT->next = NULL;

            // create head node for the terminated linked list
            terminatedH = (tcb*) malloc( sizeof( tcb ) );
            terminatedH->threadID = 0;
            terminatedH->next = NULL;
            terminatedH->prev = NULL;
        }
    
        // check if the list is empty (besides head and tail)
        if(runqueueT->prev == runqueueH){
            runqueueT->prev = node;
        }

        // set the new nodes prev and next
        node->prev = runqueueH;
        node->next = runqueueH->next;
        // change the previous first nodes data
        (node->next)->prev = node;
        // change the heads data
        runqueueH->next = node;
    }else if( queue == -1){
        // add to terminated queue
        node->prev = terminatedH;
        node->next = terminatedH->next;
        // check if the prev first terminated exists
        if(terminatedH->next){
            // if it does, change its prev to the new front
            (terminatedH->next)->prev = node;
        }
        // change the head of terminated
        terminatedH->next = node;
    }
    return;
}

tcb* dequeue(int queue){
    // what queue are we dequeing from
    if( queue == 0 ){
        // runqeueu
        // check if there is anything in the list
        if( runqueueT->prev == runqueueH ){
            // there is nothing in the list to dequeue
            return NULL;
        }

        // get access to the node we want to dequeue
        tcb* node = runqueueT->prev;
        // change values of node's prev
        (node->prev)->next = node->next;
        // change values of tail
        runqueueT->prev = node->prev;
        // set node's values to NULL                DO I NEED THIS?
        node->prev = NULL;
        node->next = NULL;

        return node;
    }else if( queue == -1 ){
        // terminated queue
        if( terminatedH->next == NULL )
            return NULL;
        tcb* node = terminatedH;
        // get to the last terminated node
        while( node->next != NULL ){
            node = node->next;
        }
    
        // if only node is the head return null
        if(terminatedH == node)
            return NULL;

        return node;

    }
    return NULL;
};

tcb* findTCB( rpthread_t thread, int queue ){
    // search through our runqueue
    //if( queue == 0 ){
        //puts("findTCB(): looking in runqueue");
    tcb* temp = runqueueH;
    while(temp){
        if(temp->threadID == thread){
            return temp;
        }
        temp = temp->next;
    }
    //}else if( queue == -1 ){
        // search through our terminated
        //puts("findTCB(): looking in terminated");
        //tcb* temp = terminatedH;
    temp = terminatedH;
    while(temp){
        if(temp->threadID == thread)
            return temp;
        temp = temp->next;
    }
    //}
    //puts("findTCB(): uh oh found in neither");
    printList();
    puts("");
    return NULL;
}

// returns 1 if it is empty
// returns 0 if it is not empty
int isEmpty(){
    return runqueueH->next == runqueueT;
}

void sig_handler(int signum){
    if(signum == SIGPROF){
        puts("sighand(): Switching back to sched");
        swapcontext( &(currThread->context), &schedCont );
    }
}
