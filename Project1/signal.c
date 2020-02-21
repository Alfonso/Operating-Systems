/* signal.c
 *
 * Group Members Names and NetIDs:
 *   1. Alfonso Buono ajb393
 *   2. Ali Mohamad aam345
 *
 * ILab Machine Tested on: kill
 *
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/* Part 1 - Step 2 to 4: Do your tricks here
 * Your goal must be to change the stack frame of caller (main function)
 * such that you get to the line after "r2 = *( (int *) 0 )"
 */
void segment_fault_handler(int signum) {

    printf("I am slain!\n");

    *(&signum + 47) += 2;

}

int main(int argc, char *argv[]) {

    int r2 = 0;

    signal(SIGSEGV, segment_fault_handler);
    
    r2 = *( (int *) 0 );

    printf("I live again!\n");

    return 0;
}
