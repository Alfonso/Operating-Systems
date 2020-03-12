#include <stdio.h>
#include <stdlib.h>

int lock;

int main(int argc, char** argv){

    lock = 0;
    int old = -1;
    
    old = __sync_lock_test_and_set(&lock, 1);

    __sync_lock_test_and_set(&lock, 2);

    printf("old: %d\n",old);

    printf("lock: %d\n",lock);

}
