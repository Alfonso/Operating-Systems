#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

struct itimerval timer;

void ring(int signum){
    printf("RING RING! The timer has gone off\n");
    setitimer(ITIMER_PROF, &timer, NULL);
}

int main(int argc,char** argv){

    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &ring;
    sigaction (SIGPROF, &sa, NULL);


    timer.it_value.tv_usec = 0;
    timer.it_value.tv_sec = 2;

    setitimer(ITIMER_PROF, &timer, NULL);

    while(1);

}
