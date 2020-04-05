#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void cpy(void* dest, void* src, int size){
    memcpy(dest,src,size);
}

void cpyHalf(void* dest, void* src, int size){
    memcpy(dest, src + size/2,size/2);
}

int main(int argc,char** argv){

    int buffer[100] = {0};

    buffer[0] = 1;
    buffer[99] = 5;
    buffer[50] = 10;

    int* test = malloc( sizeof(int) * 100 );
    
    cpyHalf( (void*) test, (void*) buffer, sizeof(int) * 100 );

    printf("Idx 0: %d, Idx 50: %d, Idx 99: %d\n",test[0],test[50],test[99]);

}
