#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>


void* printThread(void* args){
    printf("pthread_id: %u\n", pthread_self() );
    fflush(stdout);
    return;
}

int main(int argc, char** arv){

    pthread_t thread;
    pthread_create(&thread,NULL, printThread,NULL);
    printf("pthread_t: %u\n",thread);
    pthread_join( thread, NULL );
}
