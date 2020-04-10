#include "my_vm.c"

void checkFree(){

    printf("a_malloc() call. Success\n");
    void* testPointer = a_malloc(4097);
    
    printf("a_free() call. Success\n");
    a_free( testPointer, 4097 );

    printf("\na_free() call. Error. Freeing pointer twice\n");
    a_free( testPointer, 4097 );

    printf("\na_malloc() call. Success\n");
    void* testPointer2 = a_malloc(4096 * 4);

    printf("a_free() call. Success\n");
    a_free( testPointer2, (4096*4) );
}

void checkNonContigPhys(){

    printf("a_malloc() call. Success\n");
    void* testPointer = a_malloc(1);

    printf("\na_malloc() call. Success\n");
    void* testPointer2 = a_malloc(1);

    printf("\na_malloc() call. Success\n");
    void* testPointer3 = a_malloc(1);
    
    printf("\na_free() call on ptr2. success\n");
    a_free(testPointer2, 1);

    printf("\na_malloc() call. Success. Should be no contig bits\n");
    testPointer2 = a_malloc(4096 * 2);
   
    printf("\na_malloc() call. Success. Should be virt bit 2\n");
    void* testPointer4 = a_malloc(1);

}

void checkMultipleTables(){

    printf("a_malloc() call. Success. Use 1023 pages\n");
    void* pointer = a_malloc( 4096 * 1022 );

    printf("a_malloc() call. Success. Should start on 0 page table and go to 1 page table\n");
    void* pointer2 = a_malloc( 4096 * 2 );
    
}

void checkPutGet(){

    int numInts = 1025;    

    int size = numInts * sizeof(int);


    int buffer[1025] = {0};

    buffer[0] = 1;
    buffer[24] = 2;
    buffer[49] = 3;
    buffer[74] = 4;
    buffer[99] = 5;
    buffer[1000] = 6;
    buffer[1024] = 7;
    

    printf("test(): mallocing\n");
    int* test = (int*) a_malloc( size );

    printf("\ntest(): puttingng\n");
    put_value( (void*) test, (void*) buffer, size );

    int buffer2[1025] = {0};

    printf("\ntest(): getting\n");
    get_value( (void*) test, (void*) buffer2, size );

    printf("idx 0: %d, idx 24: %d, idx 49: %d, idx 74: %d, idx 99: %d, idx 1000: %d, idx 1024: %d\n",buffer2[0],buffer2[24],buffer2[49],buffer2[74],buffer2[99],buffer2[1000],buffer2[1024]);

}

void checkFree2(){

    printf("a_malloc() call. Success\n\n");
    char* ptr = (char*) a_malloc(3);


    printf("a_free() call. Success\n\n");
    a_free(ptr,3);

    char buffer[3] = {'\0'};
   
    buffer[0] = 'H';
    buffer[1] = 'i';

    
    printf("put_value() call. Fail\n");
    put_value((void*) ptr, (void*) buffer,3);


    printf("\na_free() call. Fail\n");
    a_free(ptr,3);
}

void checkPutGet2(){
    char* ptr = (char*) a_malloc(3);

    char buff[3] = {'\0'};
    buff[0] = 'H';
    buff[1] = 'i';

    char res[3] = {'\0'};
    
    put_value((void*)ptr,(void*) buff,3);
    
    get_value((void*) ptr,(void*) res,3);

    printf("buff: %s, res: %s\n",buff,res);

}

void* threadFunc(void* param){
    void* ptr;
    printf("Thread: %d, before malloc\n",(int)param);
    ptr = (void*) a_malloc(4096);
    printf("Thread: %d, after malloc\n",(int)param);
    return;
}

void checkThread(int numThreads){
    pthread_t* threads = (pthread_t*) malloc( numThreads  * sizeof(pthread_t) );

    int counter = 0;
    for(counter = 0; counter < numThreads; counter++){
        pthread_create( &threads[counter], NULL, &threadFunc, (void*)counter );
    }
    
    for(counter = 0; counter < numThreads; counter++){
        pthread_join( threads[counter], NULL );
    }

}

void* threadFunc2(void* param){

    int counter = 0;
    for(counter = 0; counter < 10; counter++)
        a_malloc(1);

}

void checkThread2(int numThreads){
    pthread_t* threads = (pthread_t*) malloc( numThreads  * sizeof(pthread_t) );

    int counter = 0;
    for(counter = 0; counter < numThreads; counter++){
        pthread_create( &threads[counter], NULL, &threadFunc2, (void*)counter );
    }

    for(counter = 0; counter < numThreads; counter++){
        pthread_join( threads[counter], NULL );
    }
   
    printFirst500(0);
    printFirst500(1);
 
}

void* threadFunc3(void* param){

    int counter = 0;
    for(counter = 0; counter < 10; counter++){
        void* ptr = a_malloc(1);
        a_free(ptr,1);
    }

}
void checkThread3(int numThreads){
    pthread_t* threads = (pthread_t*) malloc( numThreads  * sizeof(pthread_t) );

    int counter = 0;
    for(counter = 0; counter < numThreads; counter++){
        pthread_create( &threads[counter], NULL, &threadFunc3, (void*)counter );
    }

    for(counter = 0; counter < numThreads; counter++){
        pthread_join( threads[counter], NULL );
    }
 
}
void checkPGSIZE(){
    void* ptr = a_malloc( 4097 );
    void* ptr2 = a_malloc( 8193 );

    a_free(ptr, 4097);
    a_free(ptr2, 8193);

}

void checkTLB(){
    puts("First malloc");
    char* ptr = a_malloc( 3 );
    
    char buff[3] = {'\0'};
    buff[0] = 'h';
    buff[1] = 'i';

    char res[3] = {'\0'};

    puts("putting string into ptr. Should be tlb hit");
    put_value((void*) ptr, (void*) buff, 3 );

    puts("getting string from ptr and putting into res. Should be tlb hit");
    get_value( (void*) ptr, (void*) res, 3);

    puts("printing hit rate");
    print_TLB_missrate();
    

}

// make sure we split memory among hte pages correctly
void checkPageSplit(){
    // setting up so physical pages are not contig
    void* ptr = a_malloc(1);
    char* ptr2 = a_malloc(3);
    
    a_free(ptr, 1);

    // store 3 bytes into ptr2
    char buff[3] = {'\0'};
    buff[0] = 'H';
    buff[1] = 'i';
    char res[3] = {'\0'};

    // test if we are putting the string into ptr2 and getting it
    puts("Putting buff into ptr2");
    put_value( (void*) ptr2, (void*) buff, 3);
    get_value( (void*) ptr2, (void*) res, 3 );
    printf("buff: %s, res: %s\n",buff, res);

    // now malloc two pages
    int* intArr = a_malloc( 4098 );

    int x = 10;

    puts("\nJust getting to get the original phys addres");
    put_value( (void*) intArr, (void*) buff, 1);
    
    // trying to put val into int arr
    puts("\nTrying to put val into intArr");
    put_value( (void*) ( ( (unsigned long) intArr) + 4094), &x, 4 );


    puts("\nTrying to get chars from ptr2");
    // check to see if we went past the physical page and messed stuff up
    get_value( (void*) ptr2, (void*) res, 3 );
    printf("buff: %s, res: %s\n",buff, res);

    int y = 0;
    puts("\nTrying to see if the int was properly stored in intArr");
    printf("x: %d, y: %d\n", x, y);   
    get_value( (void*) (((unsigned long) intArr) + 4094), &y, 4);
    printf("x: %d, y: %d\n", x, y);   

}

int main(int argc, char** argv){
    checkNonContigPhys();
}
