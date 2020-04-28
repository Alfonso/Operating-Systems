#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char** argv){

    char path[50] = "/tmp/ajb393/mountdir";

    char* token;
    
    token = strtok(path,"/");
    while( token != NULL ){
        printf("token is: %s\n",token);
        token = strtok(NULL, "/");
    }

    return 0;
}
