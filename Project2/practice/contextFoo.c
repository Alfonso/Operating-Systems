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
        setcontext(&barctx);
    }else if(curContext == 2){
        setcontext(&fooctx);
    }
    
}

int main(int argc,char** argv){
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &ring;
    sigaction (SIGPROF, &sa, NULL);

    struct itimerval timer;

    timer.it_interval.tv_usec = 0; 
    timer.it_interval.tv_sec = 0;

    timer.it_value.tv_usec = 0;
    timer.it_value.tv_sec = 1;

    setitimer(ITIMER_PROF, &timer, NULL);


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
    barctx.uc_stack.ss_sp=stack;
    barctx.uc_stack.ss_size= SIGSTKSZ;
    barctx.uc_stack.ss_flags=0;
    
    makecontext(&fooctx,(void *)&foo,0);
    makecontext(&barctx,(void *)&bar,0);

    curContext = 1;

    setcontext(&fooctx);

    return 0;

}
