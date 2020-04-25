#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char** argv){
    char buffer[20] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','\0'};
    printf("buffer: %s\n",buffer);

    char str[2] = {'9','9'};

    buffer[1] = '\0';
    printf("buffer: %s\n",buffer);

    strncpy(buffer+2,str,2);
    printf("buffer: %s\n",buffer);

    buffer[1] = 'b';
    printf("buffer: %s\n",buffer);

}
