#include<stdio.h>
#include<stdlib.h>
#include"rpthread.c"

void* testFunc(void* params){

    puts("in test func");

    return 0;
}


int main(int argc,char** argv){

    rpthread_t test;
    rpthread_create(&test,NULL, testFunc,NULL);

    return 0;
}
