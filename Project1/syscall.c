
 /*   Group Member Names and NetIDs:
 *   1. Alfonso Buono ajb393
 *   2.
 *
 * ILab Machine Tested on: kill
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>                                                                                
#include <sys/syscall.h>

double avg_time = 0;

int main(int argc, char *argv[]) {

    /* Implement Code Here */

    // Remember to place your final calculated average time
    // per system call into the avg_time variable
    int counter = 0;
    
    struct timeval start, end;

    gettimeofday(&start,NULL);

    for(counter=0;counter<5000000;counter++){    
        syscall(SYS_getuid);
    }
    gettimeofday(&end,NULL);

    double elapsed = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
    avg_time = elapsed / 5000000;

    printf("Average time per system call is %0.4f microseconds\n", avg_time);
    
    return 0;
}
