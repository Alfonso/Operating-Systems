#include<stdio.h>
#include<stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <ucontext.h>
#include<string.h>

// Global contexts
ucontext_t cctx,fooctx,barctx;
// 0 original, 1 for foo, 2 for bar
int curContext = 0;

#define INTERVAL 1000


void foo(){
    for(;;){
        printf("foo\n");
    }
}

void bar(){
    for(;;){
        printf("bar\n");
    }
}

void ring(int signum){
    printf("RING RING! The timer has gone off\n");
 
    if(curContext == 1){
        curContext = 2;
        swapcontext(&fooctx, &barctx);
    }else if(curContext == 2){
        curContext = 1;
        swapcontext(&barctx, &fooctx);
    }
    
}

int main(int argc,char** argv){
    signal(SIGPROF,ring); 
    struct itimerval it_val;
    it_val.it_value.tv_sec = INTERVAL/1000;
    it_val.it_value.tv_usec = (INTERVAL*1000) % 1000000;
    it_val.it_interval = it_val.it_value;
    setitimer(ITIMER_PROF,&it_val,NULL);



    if (getcontext(&fooctx) < 0){
        perror("getcontext");
        exit(1);
    }

    if (getcontext(&barctx) < 0){
        perror("getcontext");
        exit(1);
    }
    

    void *stack=malloc( SIGSTKSZ );
    void *stackbar = malloc( SIGSTKSZ );


    if (stack == NULL){
        perror("Failed to allocate stack");
        exit(1);
    }

    if(stackbar == NULL){
        perror("failed to allocate stack");
        exit(1);
    }

    fooctx.uc_link=NULL;
    fooctx.uc_stack.ss_sp=stack;
    fooctx.uc_stack.ss_size= SIGSTKSZ;
    fooctx.uc_stack.ss_flags=0;

    barctx.uc_link=NULL;
    barctx.uc_stack.ss_sp=stackbar;
    barctx.uc_stack.ss_size= SIGSTKSZ;
    barctx.uc_stack.ss_flags=0;
    
    makecontext(&fooctx,(void *)&foo,0);
    makecontext(&barctx,(void *)&bar,0);

    curContext = 1;

    setcontext(&fooctx);

    return 0;

}
