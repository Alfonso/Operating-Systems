#include<stdio.h>
#include<stdlib.h>

void createBitArray(char** bitArr, int numBits){
    int numChars = numBits / 8;
    (*bitArr) = (char*) malloc(sizeof(char) * numChars);
    memset(*bitArr,0,numChars*sizeof(char));
    return;
}

void setBit(char* bitArr, int k){
    bitArr[k/(sizeof(char)*8)] |= 1 << (k%(sizeof(char)*8));
}

void clearBit(char* bitArr, int k){
    bitArr[k/(sizeof(char)*8)] &= ~(1 << (k%(sizeof(char)*8)));
}

int testBit(char* bitArr, int k){
    return ( (bitArr[k/(sizeof(char)*8)] & (1 << (k%(sizeof(char)*8)) )) != 0 ); 
}

int main(int argc, char** argv){

    char* bitArr;
    createBitArray(&bitArr,32);
    printf("Num is: %d\n",*((int*)bitArr));
    setBit(bitArr,0);
    printf("Num is: %d\n",*((int*)bitArr));
    setBit(bitArr,1);
    printf("Num is: %d\n",*((int*)bitArr));
    clearBit(bitArr,0);
    printf("Num is: %d\n",*((int*)bitArr));
    clearBit(bitArr,1);
    printf("Num is: %d\n",*((int*)bitArr));
    
    return 0;
}
