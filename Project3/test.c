#include "my_vm.c"

void checkFree(){

    printf("a_malloc() call. Success\n");
    void* testPointer = a_malloc(4097);
    
    printf("a_free() call. Success\n");
    a_free( testPointer, 4097 );

    printf("\na_free() call. Error\n");
    a_free( testPointer, 4097 );

    printf("\na_malloc() call. Success\n");
    void* testPointer2 = a_malloc(4096 * 4);

    printf("a_free() call. Success\n");
    a_free( testPointer2, (4096*4) );
}

void checkNonContigPhys(){

    printf("a_malloc() call. Success\n");
    void* testPointer = a_malloc(1);

    printf("a_malloc() call. Success\n");
    void* testPointer2 = a_malloc(1);

    printf("a_malloc() call. Success\n");
    void* testPointer3 = a_malloc(1);
    
    printf("a_free() call. success\n");
    a_free(testPointer2, 1);

    printf("a_malloc() call. Success. Should be no contig bits\n");
    testPointer2 = a_malloc(4096 * 2);
   
    printf("a_malloc() call. Success. Should be virt bit 2\n");
    void* testPointer4 = a_malloc(1);

}

void checkMultipleTables(){

    printf("a_malloc() call. Success. Use 1023 pages\n");
    void* pointer = a_malloc( 4096 * 1022 );

    printf("a_malloc() call. Success. Should start on 0 page table and go to 1 page table\n");
    void* pointer2 = a_malloc( 4096 * 2 );
    
}

void checkTLB(){

}

void checkPutGet(){

    int numInts = 100;    

    int size = numInts * sizeof(int);


    int buffer[100] = {0};

    buffer[0] = 1;
    buffer[24] = 2;
    buffer[49] = 3;
    buffer[74] = 4;
    buffer[99] = 5;
    
    printf("test(): mallocing\n");
    int* test = (int*) a_malloc( size );

    printf("test(): puttingng\n");
    put_value( (void*) test, (void*) buffer, size );

    int buffer2[100] = {0};

    printf("test(): getting\n");
    get_value( (void*) test, (void*) buffer2, size );

    printf("idx 0: %d, idx 24: %d, idx 49: %d, idx 74: %d, idx 99: %d\n",buffer2[0],buffer2[24],buffer2[49],buffer2[74],buffer2[99]);

}

int main(int argc, char** argv){

    checkPutGet();

}
