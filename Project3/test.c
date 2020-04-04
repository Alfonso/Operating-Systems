#include "my_vm.c"

int main(int argc, char** argv){

    printf("a_malloc() call\n");
    
    void* testPointer = a_malloc(4097);

    printf("\na_malloc() call\n");


    void* testPointer2 = a_malloc(4096 * 4);

}
