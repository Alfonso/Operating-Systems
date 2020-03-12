#include <stdio.h>
#include <stdlib.h>
#include "rpthread.c"

rpthread_mutex_t lock;
int counter = 0;

void* func1(void* args){
    puts("In func1");
    rpthread_mutex_lock(&lock);
    puts("In lock in func1");
    for(;;){
    }
    rpthread_mutex_unlock(&lock);

    rpthread_exit(NULL);
}

void* func2(void* args){
    puts("IN func2");
    rpthread_mutex_lock(&lock);
    puts("in lock in func2");
    rpthread_mutex_unlock(&lock);

    rpthread_exit(NULL);
}

void* func3(void* args){
    int x = 0;
    
    //rpthread_mutex_lock(&lock);
    
    for(x = 0; x < 50000000; x++){
        counter += 1;
    }
   
    //rpthread_mutex_unlock(&lock);
    
    pthread_exit(NULL);
}


int main(int argc, char** argv){

    rpthread_mutex_init(&lock,NULL);

    rpthread_t t1;
    rpthread_t t2;

    rpthread_create(&t1,NULL,func3,NULL);
    puts("after create t1");
    rpthread_create(&t2,NULL,func3,NULL);
    puts("After create t2");

    rpthread_join(t1,NULL);
    rpthread_join(t2,NULL);

    printf("Counter: %d\n",counter);

    return 0;
}
