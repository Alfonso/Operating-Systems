#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){

    struct inode* inode;

    indirectPtrIdx = 0;
    // loop through all of the indirect pointers
    for( indirectPtrIdx = 0; indirectPtrIdx < (sizeof(inode->indirect_ptr)/sizeof(int)); indirectPtrIdx ){
        // chcek if the pointer is valid
        if( (inode->indirect_ptr)[indirectPtrIdx] >= 0 ){
            // it is valid so read the block
            char* indirectBlockBuff = (char*) malloc(sizeof(char)*BLOCK_SIZE);
            int indirectBlockNum = (inode->indirect_ptr)[indirectPtrIdx] + sb->d_start_blk;
            bio_read( indirectBlockNum, (void*) indirectBlockBuff );
            // now loop through the indirect block in increments of our struct
            int indirectBlockIdx = 0;
            for( indirectBlockIdx = 0; indirectBlockIdx < indirectPtrBlockPerBlock; indirectBlockIdx++ ){
                char* indirect

            }
        }
    }



    return 0;
}


