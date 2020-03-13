// File:	rpthread.c

// List all group member's name: Alfonso Buono, Ali Mohamad
// username of iLab: ajb393, aam345
// iLab Server: kill

/*
 *  WE NEED TO:
 *  1) Make it so schedule picks based off of the ifndef
 *  2) make it so sig handler does based off of MLFQ in ifndef
 *  3) finish promote
 *
*/


#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

// linked list pointers.                    
tcb* runqueueH;
tcb* runqueueT;

// pointers and counter for MLFQ
tcb* firstqueueH;
tcb* firstqueueT;

tcb* secondqueueH;
tcb* secondqueueT;

tcb* thirdqueueH;
tcb* thirdqueueT;

int quantumCounter = 0;

// terminated queue
tcb* terminatedH;
// blocked Queue
tcb* blockedH;

// Global current thread counter
tcb* currThread;

// Global Schedule context
ucontext_t schedCont;
int initialSched = 0;

// thread id counter
rpthread_t threadCounter = 2;

// global timer
struct itimerval it_val;
struct itimerval it_zero;

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

    // set data in tcb
    threadBlock->threadID = threadCounter; 
    threadCounter += 1;
    threadBlock->threadStatus = ready;
    threadBlock->stack = stack;
    threadBlock->priority = 0;
    threadBlock->timeElapsed = 0;
  
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
    setitimer(ITIMER_PROF,&it_zero,NULL);
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
    // What ever you pass to pthread exit, wwhoever passes pthread_join
    if(value_ptr){
        currThread->retval = value_ptr;
    } 
 
    // turn the timer off
    setitimer(ITIMER_PROF,&it_zero,NULL);
    puts("exit(): turned timer off going back to sched");


    // return back to the scheduler?                        Do I need to do this?
    setcontext( &schedCont );

    return;
};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// Wait for a specific thread to terminate
	// De-allocate any dynamic memory created by the joining thread
	// YOUR CODE HERE
 
    // find the tcb associated to thread
    tcb* temp = findTCB( thread, -1 );
    printf("join(): searching for: %u\n",thread);
    // infinitely loop to block thread that called this until thread passed into parameters is terminated
    while( temp->threadStatus != terminated ){
    }
    puts("join(): In join after while");


    // check if user wants a retval
    if(value_ptr){
        *value_ptr = (temp->retval);
    }
/*
    // deallocate memory
    if( temp->stack )
        free( temp->stack );
*/
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//Initialize data structures for this mutex
    
	// YOUR CODE HERE
	
    if(mutex == NULL)
        return -1;
    //                                                                          DO I NEED TO MALLOC THIS
    *mutex = *((rpthread_mutex_t*) malloc(sizeof( rpthread_mutex_t )));
    
    // Create a head and a tail
    mutex->queueH = (tcb*) malloc( sizeof( tcb ) );
    mutex->queueT = (tcb*) malloc( sizeof( tcb ) );
    (mutex->queueH)->Mnext = mutex->queueT;
    (mutex->queueT)->Mprev = mutex->queueH;


    mutex->isLocked = 0;
    mutex->owner = NULL;
    
    return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
    // use the built-in test-and-set atomic function to test the mutex
    // When the mutex is acquired successfully, enter the critical section
    // If acquiring mutex fails, push current thread into block list and 
    // context switch to the scheduler thread

    // YOUR CODE HERE
    // use test and set bc its atomic (dont want the possibility of a thread switching
    while( __sync_lock_test_and_set( &(mutex->isLocked), 1 ) == 1  ){
        puts("lock(): Youre blocked bitch");

        // add the curr thread into the mutex wait list
        currThread->Mnext = mutex->queueH->Mnext;
        currThread->Mprev = mutex->queueH;
            
        // change mutex list's head ot point to the curr thread
        mutex->queueH->Mnext = currThread;
            
        // make the next's prev equal current
        (currThread->Mnext)->Mprev = currThread;
        
        // change status to blocked so scheduler adds to blocked queue
        currThread->threadStatus = blocked;
        swapcontext( &(currThread->context), &(schedCont) );
    }

    // set owner so we know if the correct thread is unlocking
    mutex->owner = currThread;

    return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	
    // check if the person calling this actually owns the mutex
    if(mutex->owner == currThread){
        // check if there is a wait queue
        if( (mutex->queueH)->Mnext != mutex->queueT){
            // pick the next thread that gets access
            tcb* temp = (mutex->queueT)->Mprev;
        
             
            // remove the next thread from mutex list
            (temp->Mprev)->Mnext = mutex->queueT;
            mutex->queueT->Mprev = temp->Mprev;

            // remove the next thread from block queue
            printf("unlock(): Blocked Queue before remove:       ");
            printBlock();
            (temp->prev)->next = temp->next;
            // check if there is something after curr
            if( (temp->next) )
                (temp->next)->prev = temp->prev;
            printf("       Before Queue after remove: ");
            printBlock();
            puts("");

            // change its status to ready and add it to the runqueue
            temp->threadStatus = ready;
            enqueue(temp, 0);
        }

        // set the flag to 0
        mutex->isLocked = 0;
        mutex->owner = NULL;
    }else{
        puts("You do not have access to this mutex");
        return -1;
    }


    return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init

    // check if the person destroying has access to mutex
    if( currThread == mutex->owner ){
        
        if(mutex == NULL){
            return EINVAL;
        }

        // free the lock?
        puts("Mutex destroyed");
        free( mutex );

    }else{
        puts("You do not have access to this mutex");
        return -1;
    }

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
    sa.sa_handler = &timer_handler;
    sigaction (SIGPROF, &sa, NULL);    

    it_val.it_value.tv_sec = INTERVAL/1000;
    it_val.it_value.tv_usec = (INTERVAL*1000) % 1000000;
    

    while( isEmpty() != 1 ){
        
        if( currThread ){
        
            if(currThread->priority == 0){
                printf("sched(): runqueue before: ");
            }else if( currThread->priority == 1){
                printf("sched(): firstqueue before: ");
            }else if( currThread->priority == 2){
                printf("sched(): secondqueue before: ");
            }else if( currThread->priority == 3){
                printf("sched(): thirdqueue before: ");
            }
            printList( currThread->priority );
        }else{
            printf("sched(): before runqueue: ");
            printList(0);
        }
        // find next thread to run
        #ifndef MLFQ
	        // Choose STCF
	        //sched = STCF;
	        sched_stcf();
        #else
	        // Choose MLFQ
	        //sched = MLFQ;
            sched_mlfq();
        #endif

        // switch its status to running
        currThread->threadStatus = running;

        printf("    Switching threads to: %u   ",currThread->threadID);
        if(currThread->priority == 0){
            printf("  runqueue after:   ");
        }else if( currThread->priority == 1){
            printf("  firstqueue after:  ");
        }else if( currThread->priority == 2){
            printf("   secondqueue after:   ");
        }else if( currThread->priority == 3){
            printf("  thirdqueue after:  ");
        }
        printList( currThread->priority );
        puts("");

        // start the timer
        setitimer(ITIMER_PROF,&it_val,NULL);
        
        // swap context
        swapcontext( &schedCont, &(currThread->context) );
    
        // increase timeElapsed by 1                                                                THIS IS FOR STCF
        currThread->timeElapsed += 1;
        
        // increase priority is done in the signal handler
        // increase global quantum timer in signal handler
        
        // do we put the thread back into the runqueue? have to check if its terminated or not?
        if( (currThread->threadStatus != terminated) && (currThread->threadStatus != blocked) ){
            currThread->threadStatus = ready;
            printf("sched(): Adding %u to queue:%d \n", currThread->threadID, currThread->priority);
            // add to queue based off of prioirty. If its not MLFQ it should just add it to 0 queue
            enqueue( currThread, currThread->priority );
        }else if(currThread->threadStatus == terminated){
            printf("sched(): Adding %u to terminated\n", currThread->threadID);
            enqueue( currThread, -1 );
        }else if(currThread->threadStatus == blocked){
            printf("sched(): Adding %u to blocked\n", currThread->threadID);
            enqueue( currThread, -2 );
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
	// need to find the shortest time elapsed thread
	tcb* min = runqueueH->next;
    tcb* ptr = runqueueH->next;
    
    while( ptr!= runqueueT ){
        if( ptr->timeElapsed <= min->timeElapsed ){
            min = ptr;
        }
        ptr = ptr->next;
    }
    // check if the queue is empty?
    if( min == runqueueT )
        return;
    // set the currThread min time
    currThread = min;
    // remove it from the runqueue
	(currThread->prev)->next = currThread->next;
    (currThread->next)->prev = currThread->prev;
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
    
    // need to check each of the queues starting from top for the queue with an element present
    if( runqueueH->next != runqueueT){
        // dequeue because there is something in top queue
        currThread = dequeue(0);
    }else if( firstqueueH->next != firstqueueT ){
        currThread = dequeue(1);
    }else if( secondqueueH->next != secondqueueT ){
        currThread = dequeue(2);
    }else if( thirdqueueH->next != thirdqueueT ){
        currThread = dequeue(3);
    }
}

// Feel free to add any other functions you need

// YOUR CODE HERE


// sets the global schedule context
// Called once
void sched_context_create(){
    initialSched = 1;
    
    // creating zero timer val;
    it_zero.it_value.tv_sec = 0;
    it_zero.it_value.tv_usec = 0;

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
void printList(int queue){
    if( queue == 0){
        tcb* ptr = runqueueH;
        while(ptr != runqueueT){
            printf("%u->",ptr->threadID);
            ptr = ptr->next;
        }
        printf("%u",ptr->threadID);
    }else if( queue == 1 ){
        tcb* ptr = firstqueueH;
        while(ptr != firstqueueT){
            printf("%u->",ptr->threadID);
            ptr = ptr->next;
        }
        printf("%u",ptr->threadID);

    }else if( queue == 2 ){
        tcb* ptr = secondqueueH;
        while(ptr != secondqueueT){
            printf("%u->",ptr->threadID);
            ptr = ptr->next;
        }
        printf("%u",ptr->threadID);

    }else if( queue == 3 ){
        tcb* ptr = thirdqueueH;
        while(ptr != thirdqueueT){
            printf("%u->",ptr->threadID);
            ptr = ptr->next;
        }
        printf("%u",ptr->threadID);

    }
}

void printBlock(){
    tcb* ptr = blockedH;
    while(ptr){
        printf("%u->",ptr->threadID);
        ptr = ptr->next;
    }
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

            // create all of the other queues for MLFQ (even if we do not use it)
            createMLFQ();

            // create head node for the terminated linked list
            terminatedH = (tcb*) malloc( sizeof( tcb ) );
            terminatedH->threadID = 0;
            terminatedH->next = NULL;
            terminatedH->prev = NULL;

            // create head node for the blocked linked list
            blockedH = (tcb*) malloc( sizeof( tcb ) );
            blockedH->threadID = 0;
            blockedH->next = NULL;
            blockedH->prev = NULL;
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
    }else if( queue == -2){
        node->prev = blockedH;
        node->next = blockedH->next;

        // check if the prev first blocked exists
        if(blockedH->next){
            (blockedH->next)->prev = node;
        }
        blockedH->next = node;
    }else if( queue == 1 ){

        // check if the list is empty (besides head and tail)
        if(firstqueueT->prev == firstqueueH){
            firstqueueT->prev = node;
        }

        // set the new nodes prev and next
        node->prev = firstqueueH;
        node->next = firstqueueH->next;
        // change the previous first nodes data
        (node->next)->prev = node;
        // change the heads data
        firstqueueH->next = node;
    }else if( queue == 2 ){

        // check if the list is empty (besides head and tail)
        if(secondqueueT->prev == secondqueueH){
            secondqueueT->prev = node;
        }

        // set the new nodes prev and next
        node->prev = secondqueueH;
        node->next = secondqueueH->next;
        // change the previous first nodes data
        (node->next)->prev = node;
        // change the heads data
        secondqueueH->next = node;
    }else if( queue == 3 ){

        // check if the list is empty (besides head and tail)
        if(thirdqueueT->prev == thirdqueueH){
            thirdqueueT->prev = node;
        }

        // set the new nodes prev and next
        node->prev = thirdqueueH;
        node->next = thirdqueueH->next;
        // change the previous first nodes data
        (node->next)->prev = node;
        // change the heads data
        thirdqueueH->next = node;
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

    }else if( queue == 1 ){
        // firstqeueu
        // check if there is anything in the list
        if( firstqueueT->prev == firstqueueH ){
            // there is nothing in the list to dequeue
            return NULL;
        }

        // get access to the node we want to dequeue
        tcb* node = firstqueueT->prev;
        // change values of node's prev
        (node->prev)->next = node->next;
        // change values of tail
        firstqueueT->prev = node->prev;
        // set node's values to NULL                DO I NEED THIS?
        node->prev = NULL;
        node->next = NULL;
        return node;
    }else if( queue == 2 ){
        // secondqeueu
        // check if there is anything in the list
        if( secondqueueT->prev == secondqueueH ){
            // there is nothing in the list to dequeue
            return NULL;
        }

        // get access to the node we want to dequeue
        tcb* node = secondqueueT->prev;
        // change values of node's prev
        (node->prev)->next = node->next;
        // change values of tail
        secondqueueT->prev = node->prev;
        // set node's values to NULL                DO I NEED THIS?
        node->prev = NULL;
        node->next = NULL;
        return node;
    }else if( queue == 3 ){
        // thirdqeueu
        // check if there is anything in the list
        if( thirdqueueT->prev == thirdqueueH ){
            // there is nothing in the list to dequeue
            return NULL;
        }

        // get access to the node we want to dequeue
        tcb* node = thirdqueueT->prev;
        // change values of node's prev
        (node->prev)->next = node->next;
        // change values of tail
        thirdqueueT->prev = node->prev;
        // set node's values to NULL                DO I NEED THIS?
        node->prev = NULL;
        node->next = NULL;
        return node;
    }
    return NULL;
};

/*  returns the tcb that corresponds to threadID    */
tcb* findTCB( rpthread_t thread, int queue ){
    //if( queue == 0 ){
    // search through our runqueue
    tcb* temp = runqueueH;
    while(temp){
        if(temp->threadID == thread){
            return temp;
        }
        temp = temp->next;
    }
    // search through our runqueue
    temp = firstqueueH;
    while(temp){
        if(temp->threadID == thread){
            return temp;
        }
        temp = temp->next;
    }
    // search through our runqueue
    temp = secondqueueH;
    while(temp){
        if(temp->threadID == thread){
            return temp;
        }
        temp = temp->next;
    }
    // search through our runqueue
    temp = thirdqueueH;
    while(temp){
        if(temp->threadID == thread){
            return temp;
        }
        temp = temp->next;
    }
    // search through our terminated
    temp = terminatedH;
    while(temp){
        if(temp->threadID == thread)
            return temp;
        temp = temp->next;
    }
    // search through blocked
    temp = blockedH;
    while(temp){
        if( temp->threadID == thread)
            return temp;
        temp = temp->next;
    }
    //}
    //puts("findTCB(): uh oh found in neither");
    return NULL;
}

// returns 1 if it is empty
// returns 0 if it is not empty
int isEmpty(){
    if( runqueueH->next != runqueueT ){
        return 0;
    }
    if( firstqueueH->next != firstqueueT ){
        return 0;
    }
    if( secondqueueH->next != secondqueueT ){
        return 0;
    }
    if( thirdqueueH->next != thirdqueueT ){
        return 0;
    }

    return 1;
}

void timer_handler(int signum){
    if(signum == SIGPROF){

        #ifndef MLFQ
        #else
            if(currThread->priority < 3){
                currThread->priority += 1;
            }
            quantumCounter += 1;
            if(quantumCounter > 9){
                puts("sighand(): promoting");
                promote();
                quantumCounter = 0;
            }
        #endif

        puts("sighand(): Switching back to sched");
        swapcontext( &(currThread->context), &schedCont );
    }
}

void createMLFQ(){
    // first priority queue 
    firstqueueH = (tcb*) malloc( sizeof( tcb ) );
    firstqueueT = (tcb*) malloc( sizeof( tcb ) );

    firstqueueH->threadID = 0;
    firstqueueH->next = firstqueueT;
    firstqueueH->prev = NULL;

    firstqueueT->threadID = 0;
    firstqueueT->prev = firstqueueH;
    firstqueueT->next = NULL;

    // second priority queue
    secondqueueH = (tcb*) malloc( sizeof( tcb ));
    secondqueueT = (tcb*) malloc( sizeof( tcb ));

    secondqueueH->threadID = 0;
    secondqueueH->next = secondqueueT;
    secondqueueH->prev = NULL;

    secondqueueT->threadID = 0;
    secondqueueT->prev = secondqueueH;
    secondqueueT->next = NULL;

    // third priority queue
    thirdqueueH = (tcb*) malloc( sizeof( tcb ));
    thirdqueueT = (tcb*) malloc( sizeof( tcb ));

    thirdqueueH->threadID = 0;
    thirdqueueH->next = thirdqueueT;
    thirdqueueH->prev = NULL;

    thirdqueueT->threadID = 0;
    thirdqueueT->prev = thirdqueueH;
    thirdqueueT->next = NULL;
}

// promotes all of the threads to top queue
void promote(){                     
   
    tcb* tempH = NULL;
    tcb* tempT = NULL;

    // start at top queue
    if( firstqueueH->next != firstqueueT ){
        tempT = firstqueueT->prev;
        tempH = firstqueueH->next;

        tempT->next = runqueueH->next;
        runqueueH->next = tempH;
        tempH->prev = runqueueH;
        (tempT->next)->prev = tempT;
        firstqueueH->next = firstqueueT;
        firstqueueT->prev = firstqueueH;

    }

    if( secondqueueH->next != secondqueueT ){
        tempT = secondqueueT->prev;
        tempH = secondqueueH->next;

        tempT->next = runqueueH->next;
        runqueueH->next = tempH;
        tempH->prev = runqueueH;
        (tempT->next)->prev = tempT;
        secondqueueH->next = secondqueueT;
        secondqueueT->prev = secondqueueH;
    }

    if( thirdqueueH->next != thirdqueueT ){
        tempT = thirdqueueT->prev;
        tempH = thirdqueueH->next;

        tempT->next = runqueueH->next;
        runqueueH->next = tempH;
        tempH->prev = runqueueH;
        (tempT->next)->prev = tempT;
        thirdqueueH->next = thirdqueueT;
        thirdqueueT->prev = thirdqueueH;
    }

    // traverse through runqueue now changing all of their priorities to 1
    tcb* ptr = runqueueH->next;
    while(ptr!=runqueueT){
        ptr->priority = 0;
        ptr = ptr->next;
    }
    // change priorities of blocked threads
    ptr = blockedH->next;
    while(ptr){
        ptr->priority = 0;
        ptr = ptr->next;
    }
    // change priority of curr thread (since not in a queue)
    currThread->priority = 0;


    printf("promote(): NEW RUNQUEUE: ");
    printList(0);
    puts("");


}
