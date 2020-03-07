#include<stdio.h>
#include<stdlib.h>
#include"rpthread.c"

void* testFunc(void* params){

    puts("in test func");

    return 0;
}


int main(int argc,char** argv){

    rpthread_t test, test2, test3;

    rpthread_create(&test,NULL, testFunc,NULL);
    printf("first list: ");
    printList();
    rpthread_create(&test2,NULL,testFunc,NULL);
    printf("second list: ");
    printList();
    rpthread_create(&test3,NULL,testFunc,NULL);
    printf("third list: ");
    printList();

    printf("%d\n",isEmpty());


    return 0;
}
