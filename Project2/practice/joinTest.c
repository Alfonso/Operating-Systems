#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

void* func(void* args){
    puts("test");

    pthread_exit( (void*) 15 );

}

int main(int argc,char** argv){
    pthread_t t1;
    pthread_create( &t1,NULL, func, NULL);

    void* retval;

    pthread_join( t1, &retval);

    printf("This is retval: %d\n", ((int*) retval));

    return 0;
}
