#include<stdio.h>
#include<stdlib.h>
#include"rpthread.c"

void* testFunc(void* params){

    puts("main(): in test func");

    //while(1){
        //puts("testFunc1");
    //}
    rpthread_exit(NULL);
    return 0;
}

void* testFunc2(void* params){

    puts("main(): in test func2");

    //while(1){
        //puts("testFunc2");
    //}
    rpthread_yield();
    
    rpthread_exit(NULL);
    return 0;
}

void* testFunc3(void* params){
    puts("main(): in test func3");

    while(1){

    }
    return 0;
}


int main(int argc,char** argv){

    rpthread_t test, test2, test3;

    puts("main(): beore 1st create");
    rpthread_create(&test,NULL, testFunc,NULL);
    puts("main(): after 1st create");

    rpthread_create(&test2,NULL,testFunc2,NULL);
    puts("main(): after 2nd create");
    
    rpthread_join(test,NULL);

    puts("main(): we are after 1st join");
    
    rpthread_join(test2,NULL);

    while(1){

    }

/*
 * Test to see if linked list is working
    printf("first list: ");
    printList();
    rpthread_create(&test2,NULL,testFunc,NULL);
    printf("second list: ");
    printList();
    rpthread_create(&test3,NULL,testFunc,NULL);
    printf("third list: ");
    printList();

    printf("%d\n",isEmpty());
*/

    return 0;
}
