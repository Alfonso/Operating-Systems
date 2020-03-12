#include <stdio.h>
#include <stdlib.h>
#include "rpthread.c"

void* func1(void* args){
    puts("in func1");

    rpthread_exit((void*)15);
}


int main(int argc, char** argv){

    rpthread_t t1;

    rpthread_create(&t1,NULL,func1,NULL);

    void** retval = (void**) malloc(sizeof(void*));
    
    puts("before join");

    rpthread_join(t1, retval);

    puts("before print");

    printf("%d\n", *retval);

    return 0;
    
}
