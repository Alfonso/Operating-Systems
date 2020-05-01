/*
 *  Copyright (C) 2020 CS416 Rutgers CS
 *	Tiny File System
 *	File:	tfs.c
 *
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <libgen.h>
#include <limits.h>

#include "block.h"
#include "tfs.h"

char diskfile_path[PATH_MAX];

/*                  GLOBALS                 */
int inodesPerBlock = BLOCK_SIZE / sizeof(struct inode);
int direntsPerBlock = BLOCK_SIZE / sizeof(struct dirent);
struct superblock* sb;

// Declare your in-memory data structures here

/* 
 * Get available inode number from bitmap
 */
int get_avail_ino() {
    
    
	// Step 1: Read inode bitmap from disk
    
    /*                  CAN WE ASSUME THAT BITMAP IS ON 2ND BLOCK(idx 1)                   */
    char* buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
    char* ibuff = (char*) malloc( sizeof(char) * (MAX_INUM / 8));
    bio_read(1, (void*) buffer);
    
    // copy first bytes to ibuff
    int counter = 0;
    for(counter = 0; counter < MAX_INUM / 8; counter++){
        ibuff[counter] = buffer[counter];
    }

    // cast bitmap
    bitmap_t ibit = (bitmap_t) ibuff;

	// Step 2: Traverse inode bitmap to find an available slot
    for(counter = 0; counter < MAX_INUM; counter++){
        if( get_bitmap(ibit,counter) == 0 ){
            // counter is now the ino that is fre
	        // Step 3: Update inode bitmap and write to disk 
            set_bitmap(ibit,counter); 
            bio_write(1,(void*) ibit);
            // return the found node
            return counter;
        }
    }
    // did not find a node
	return -1;
}

/* 
 * Get available data block number from bitmap
 */
int get_avail_blkno() {

	// Step 1: Read data block bitmap from disk
    /*                  CAN WE ASSUME THAT BITMAP IS ON 3RD BLOCK (IDX 2)                   */
    /*                  CAN WE ASSUME THAT BITMAP IS ON 3RD BLOCK (IDX 2)                   */
    char* dbuff = (char*) malloc( sizeof(char) * (MAX_DNUM / 8));
    char* buffer = (char*) malloc( sizeof(char) * BLOCK_SIZE);
    bio_read(2,(void*) buffer);
    
    // copy first bytes to dbuff
    int counter = 0;
    for(counter = 0; counter < MAX_DNUM / 8; counter++){
        dbuff[counter] = buffer[counter];
    }
	
    // cast bitmap
    bitmap_t dbit = (bitmap_t) dbuff;
	
    // Step 2: Traverse data block bitmap to find an available slot
    for(counter = 0; counter < MAX_DNUM; counter++){
        if( get_bitmap(dbit,counter) == 0 ){
            // counter is now the ino that is fre
	        // Step 3: Update data block bitmap and write to disk 
            set_bitmap(dbit,counter); 
            bio_write(2,(void*) dbit);
            // memset the data block
            char* newBuff = (char*) malloc(sizeof(char) * BLOCK_SIZE);
            memset(newBuff, 0, BLOCK_SIZE);
            bio_write( counter + sb->d_start_blk, (void*) newBuff );
            // return the block number
            return counter;
        }
    }

	return -1;
}

/* 
 * inode operations
 */
int readi(uint16_t ino, struct inode *inode) {

    // Step 1: Get the inode's on-disk block number
    // in relation to the inode table
    int blockNum = ino / inodesPerBlock;
  
    // Step 2: Get offset of the inode in the inode on-disk block
    int blockOffset = ino % inodesPerBlock;

    // Step 3: Read the block from disk and then copy into inode structure
    int diskBlock = blockNum + sb->i_start_blk;
    char buffer[BLOCK_SIZE];
    
    /*                  NEED TO CHECK IF THIS FAILS                 */
    bio_read(diskBlock, (void*) buffer);
    
    char* ibuff = (char*) malloc(sizeof(struct inode));
    int counter = 0;
    for(counter = 0; counter < sizeof(struct inode); counter++){
        ibuff[counter] = buffer[blockOffset * sizeof(struct inode) + counter];
    }

    /*                 WHY DOES THIS WAY NOT WORK?                 */
    //inode = ((struct inode*) ibuff);
    /*                  SO WE HAVE TO DO THEM ONE BY ONE            */
    inode->ino = ((struct inode*) ibuff)->ino;
    inode->valid = ((struct inode*) ibuff)->valid;
    inode->size = ((struct inode*) ibuff)->size;
    inode->type = ((struct inode*) ibuff)->type;
    inode->link = ((struct inode*) ibuff)->link;
    /*                  Do we need to loop through dat?                 */
    memcpy(inode->direct_ptr,((struct inode*) ibuff)->direct_ptr,sizeof(inode->direct_ptr));
    memcpy(inode->indirect_ptr,((struct inode*) ibuff)->indirect_ptr,sizeof(inode->indirect_ptr));
    inode->vstat = ((struct inode*) ibuff)->vstat;
    
    /*                  test                    */
    //printf("readi(): NODE: ino: %d, valid: %d ,type: %d\n",inode->ino,inode->valid,inode->type);

	return 0;
}

int writei(uint16_t ino, struct inode *inode) {

	// Step 1: Get the block number where this inode resides on disk
    // in relation to the inode table
    int blockNum = ino / inodesPerBlock;
	
	// Step 2: Get the offset in the block where this inode resides on disk
    int blockOffset = ino % inodesPerBlock;

	// Step 3: Write inode to disk 
    // create a buffer to move the data to offset in buffer
    char buffer[BLOCK_SIZE];
    // calcualte where in disk
    int diskBlock = blockNum + sb->i_start_blk;
    

    /*                  ERROR CHECK THIS                    */
    // read the data already in that block
    bio_read(diskBlock,(void*) buffer);
    
    // add new inode / inode changes to buffer
    char* ibuff = (char*) inode;
    int counter = 0;
    for(counter = 0; counter < sizeof(struct inode); counter++){
        buffer[blockOffset * sizeof(struct inode) + counter] = ibuff[counter];
    }

    // write the block back to file
    bio_write(diskBlock, (void*) buffer);

	return 0;
}


/* 
 * directory operations
 * returns 0 on success -1 on failure
 */
int dir_find(uint16_t ino, const char *fname, size_t name_len, struct dirent *dirent) {

    puts("I AM IN DIR FIND!!!!!!!!!!!!!!!!!!");

    // Step 1: Call readi() to get the inode using ino (inode number of current directory)
    struct inode* inode = (struct inode*) malloc(sizeof(struct inode));
    readi(ino,inode);

    // Step 2: Get data block of current directory from inode
    int directPtrIdx = 0;
    int blockIdx = 0;
    // loop through all of the ptrs
    for(directPtrIdx = 0; directPtrIdx < (sizeof(inode->direct_ptr) / sizeof(int)); directPtrIdx++){
        // if it actually points to something
        if( (inode->direct_ptr)[directPtrIdx] >= 0 ){
        
            // copy whole block into buffer
            char* buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
            int blockNum = (inode->direct_ptr)[directPtrIdx] + sb->d_start_blk;
            bio_read( blockNum, (void*) buffer);
        
            // loop through all of the dirents in a block
            for(blockIdx = 0; blockIdx < direntsPerBlock; blockIdx++){
                // we want to only copy the bytes of a dirent into this buffer
                char* dbuff = (char*) malloc(sizeof(char) * sizeof(struct dirent));
                int direntIdx = 0;
                // copy byte by byte to our dirent buffer
                for(direntIdx = 0; direntIdx < sizeof(struct dirent); direntIdx++){
                    dbuff[direntIdx] = buffer[direntIdx + blockIdx * sizeof(struct dirent)];
                }

                // check if the dirent is valid
                if(((struct dirent*) dbuff)->valid == 0){
                    // it is not so continue
                    continue;
                }
                // check if the name is the same
                if( strcmp( ((struct dirent*) dbuff)->name, fname ) == 0 ){
                    // they are the same so copy all data into our dirent parameter and return
                    dirent->ino = ((struct dirent*) dbuff)->ino;
                    dirent->valid = ((struct dirent*) dbuff)->valid;
                    dirent->len = ((struct dirent*) dbuff)->len;
                    strcpy( dirent->name, ((struct dirent*) dbuff)->name );
                    return 0;
                }
                // they are not the same, move onto next dirent
            }
        }
    }

    // Step 3: Read directory's data block and check each directory entry.
    //If the name matches, then copy directory entry to dirent structure

    // return -1 because it failed to find it
	return -1;
}

/*
 *Returns:
 * 0 on success
 * -1 on failure
*/
int dir_add(struct inode dir_inode, uint16_t f_ino, const char *fname, size_t name_len) {

    printf("I AM IN DIR ADD AND THE FILE I AM ADDING IS: %s\n",fname);


	// Step 1: Read dir_inode's data block and check each directory entry of dir_inode
	// Step 2: Check if fname (directory name) is already used in other entries
 
    struct dirent* dirent = (struct dirent*) malloc(sizeof(struct dirent));
    int inUse = dir_find(dir_inode.ino, fname, name_len,dirent);

    if( inUse == 0 ){
        // name is in use
        puts("name is in use");
        return -1;
    }

	// Step 3: Add directory entry in dir_inode's data block and write to disk
    int directPtrIdx = 0;
    // keep track of the first non valid block
    int firstPtr = -1;
    // keep track if we wrote the dirent to the block
    int wroteDir = 0;
    // loop through all of the valid blocks
    for(directPtrIdx = 0; directPtrIdx < (sizeof(dir_inode.direct_ptr) / sizeof(int)); directPtrIdx++){
        // check if the curr index points to a block
        if( dir_inode.direct_ptr[directPtrIdx] >= 0 ){
            // now we need to loop through all of the dirents
            int blockIdx = 0;
            char* buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
            int blockNum = (dir_inode.direct_ptr)[directPtrIdx] + sb->d_start_blk;
            bio_read( blockNum, (void*) buffer);
            
            // loop through them
            for(blockIdx = 0; blockIdx < direntsPerBlock; blockIdx++){
                // we want to only copy the bytes of a dirent into this buffer
                char* dbuff = (char*) malloc(sizeof(char) * sizeof(struct dirent));
                int direntIdx = 0;
                // copy byte by byte to our dirent buffer
                for(direntIdx = 0; direntIdx < sizeof(struct dirent); direntIdx++){
                    dbuff[direntIdx] = buffer[direntIdx + blockIdx * sizeof(struct dirent)];
                }

                // check if the dirent is valid
                if(((struct dirent*) dbuff)->valid == 0){
                    // it is not so fill it here
                    wroteDir = 1;
                    ((struct dirent*) dbuff)->valid = 1;
                    ((struct dirent*) dbuff)->ino = f_ino;
                    ((struct dirent*) dbuff)->len = name_len;
                    strcpy( ((struct dirent*) dbuff)->name, fname);
                    // put this back into the block buffer
                    for(direntIdx = 0; direntIdx < sizeof(struct dirent); direntIdx++){
                        buffer[direntIdx + blockIdx * sizeof(struct dirent)] = dbuff[direntIdx];
                    }
                    // write this block back to disk
                    bio_write( blockNum, (void*) buffer);
                    // we are done with this so break out
                    break;
                }
            }
            
        }else{
            if( firstPtr == -1 ){
                // keeping track of that first non valid block index
                firstPtr = directPtrIdx;
            }
        }
        // if we already wrote dir to file we can break out
        if( wroteDir == 1 )
            break;
    }

	// Allocate a new data block for this directory if it does not exist
    // check if we wrote the dir (we only did if there was a free dirent)
    if( wroteDir == 0 ){
        // check if we have a free ptr
        if( firstPtr == -1 ){
            // we dont have any, and have not already written the dirent
            // thus we have no more space
            puts("No more space in directory");
            return -1;
        }

        // we have the first free ptr idx that we can allocate a new block for
        // find the next available data block (idx starting from data segment)
        int newDataBlock = get_avail_blkno();
        if(newDataBlock == -1){
            // no free data block
            return -1;
        }
        // make ptr point to new data block
        dir_inode.direct_ptr[firstPtr] = newDataBlock;
        // add data
        struct dirent* tempDir = (struct dirent*) malloc( sizeof(struct dirent) );
        tempDir->ino = f_ino;
        tempDir->valid = 1;
        tempDir->len = name_len;
        strcpy( tempDir->name, fname);
        // cast it to a buffer
        char* dbuff = (char*) tempDir;
        // create a block buffer and add this
        char* buffer = (char*) malloc( sizeof(char*) * BLOCK_SIZE);
        // copy over dirent to block buffer
        int counter = 0;
        for(counter = 0; counter < sizeof(struct dirent); counter++){
            buffer[counter] = dbuff[counter];
        }
        // write it to the disk (newDataBlock is 0 indexed from data segment. Have to incldue blocks before)
        bio_write( newDataBlock + sb->d_start_blk, (void*) buffer);
    }

	// Update directory inode
    // UDATE SIZE                                                       ********


	// Write directory entry to disk
    writei(dir_inode.ino, &dir_inode);

	return 0;
}

/*
 *Returns:
 * 0 on success
 * -1 on failure
*/
int dir_remove(struct inode dir_inode, const char *fname, size_t name_len) {

	// Step 1: Read dir_inode's data block and checks each directory entry of dir_inode
	
	// Step 2: Check if fname exist
    // call dir_find() to see if the file/directory exists
    struct dirent* dirent = (struct dirent*) malloc(sizeof(struct dirent));
    int inUse = dir_find(dir_inode.ino, fname, name_len, dirent);
    
    if( inUse == -1 ){
        puts("Name is not in use");
        return -1;
    }

	// Step 3: If exist, then remove it from dir_inode's data block and write to disk
    int directPtrIdx = 0;
    int blockIdx = 0;
    // loop through all the pointers to find which block and where in that block
    // this dirent exists
    for(directPtrIdx = 0; directPtrIdx < (sizeof(dir_inode.direct_ptr) / sizeof(int)); directPtrIdx++){
        // if it actually points to something
        if( (dir_inode.direct_ptr)[directPtrIdx] >= 0 ){
            
            // copy the whole block into our buffer
            char* buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
            int blockNum = (dir_inode.direct_ptr)[directPtrIdx] + sb->d_start_blk;
            bio_read( blockNum, (void*) buffer );

            // loop through all of the dirents inside this block
            for( blockIdx = 0; blockIdx < direntsPerBlock; blockIdx++){
                // we want to only copy the bytes of a dirent into this buffer
                char* dbuff = (char*) malloc(sizeof(char) * sizeof(struct dirent));
                int direntIdx = 0;
                // copy byte by byte to our dirent buffer
                for(direntIdx = 0; direntIdx < sizeof(struct dirent); direntIdx++){
                    dbuff[direntIdx] = buffer[direntIdx + blockIdx * sizeof(struct dirent)];
                }

                // check if the dirent is valid
                if( ((struct dirent*) dbuff)->valid == 0 ){
                    // it is not valid so continue to try and find another dirent
                    continue;
                }
                
                // check if the name is the same
                if( strcmp( ((struct dirent*) dbuff)->name, fname ) == 0 ){
                    // they are the same so "delete" this dirent by setting valid
                    ((struct dirent*) dbuff)->valid = 0;
                    // copy this change back to the block buffer
                    for(direntIdx = 0 ; direntIdx < sizeof(struct dirent); direntIdx++){
                        buffer[direntIdx + blockIdx * sizeof(struct dirent)] = dbuff[direntIdx];
                    }
                    // write the block buffer back to the disk
                    bio_write( blockNum, (void*) buffer);
                    
                    // update the directory inode info?                                     *******
                    // SOMETHING ABOUT ITS SIZE
                    // write this directory inode back to disk

                    // return 0 for success
                    return 0;
                }
            }
        }
    }
    

    // it did not find the dirent?
	return -1;
}

/* 
 * namei operation
 * returns:
 * 0 on success
 * -1 on failure
 */
int get_node_by_path(const char *path, uint16_t ino, struct inode *inode) {

    puts("I AM IN GET NODE BY PATH!!!!!!!!!!!!!!");
	
	// Step 1: Resolve the path name, walk through path, and finally, find its inode.
	// Note: You could either implement it in a iterative way or recursive way

    
    // create copy of the path
    char tempPath[PATH_MAX];
    strcpy(tempPath,path);
    
    // tokenize the path by forward slashes
    char* token;
    token = strtok(tempPath,"/");
    // we want to keep return value of dir_find so we can check if it was found
    int findRes = 0;
    // we want to keep the ino of the current directory
    int curIno = ino;
    // we need to keep getting the ino of each subsequent dirent in the curr directory
    struct dirent* dirent = (struct dirent*) malloc(sizeof(struct dirent));

    // loop through the whole path
    while( token != NULL ){
        // try to find the dirent
        findRes = dir_find( curIno, token,strlen(token),dirent);
        // check if it is found in curr directory
        if( findRes == -1 ){
            printf("%s is not found\n",token);
            return -1;
        }
        // change the "root" directory
        curIno = dirent->ino;
        // update the "root" directory name
        token = strtok(NULL, "/");
    }

    // get the inode data
    readi(curIno, inode);

	return 0;
}

/* 
 * Make file system
 */
int tfs_mkfs() {

	// Call dev_init() to initialize (Create) Diskfile
    dev_init(diskfile_path);


	// write superblock information
    sb = (struct superblock*) malloc( sizeof(struct superblock) );
    sb->magic_num = MAGIC_NUM;
    sb->max_inum = MAX_INUM;
    sb->max_dnum = MAX_DNUM;

    /*                  IS THIS RIGHT?                  */
    sb->i_bitmap_blk = 1;
    sb->d_bitmap_blk = 2;
    sb->i_start_blk = 3;
    sb->d_start_blk = 67;
    /*                  IS THIS RIGHT?                  */
    bio_write(0,(void*) sb);

	// initialize inode bitmap
    bitmap_t ibit = (bitmap_t) malloc( sizeof(char) * MAX_INUM / 8);
    // set them to 0
    bzero(ibit,MAX_INUM / 8);
    
	// initialize data block bitmap
    bitmap_t dbit = (bitmap_t) malloc( sizeof(char) * MAX_DNUM / 8);
    // set them to 0
    bzero(dbit,MAX_DNUM / 8);

	// update bitmap information for root directory
    set_bitmap(ibit,0);

	// update inode for root directory
    struct inode* rootI = (struct inode*) malloc( sizeof(struct inode) );
    /*                  IS THIS RIGHT?                  */
    rootI->ino = 0;
    rootI->valid = 1;
    rootI->size = 0;
    rootI->type = 1;
    rootI->link = 1;
    // loop through all values and set them to -1
    int counter = 0;
    for(counter = 0; counter < (sizeof(rootI->direct_ptr) / sizeof(int)); counter++){
        (rootI->direct_ptr)[counter] = -1;
    }
    for(counter = 0; counter < (sizeof(rootI->indirect_ptr) / sizeof(int)); counter++){
        (rootI->indirect_ptr)[counter] = -1;
    }
    // set the vstat values
    (rootI->vstat).st_ino = 0;
    (rootI->vstat).st_mode = S_IFDIR | 0755;
    (rootI->vstat).st_uid = getuid();
    (rootI->vstat).st_gid = getgid();
    (rootI->vstat).st_nlink = 1;
    (rootI->vstat).st_size = 0;
	time( &((rootI->vstat).st_atime) );
    
    /*                  IS THIS RIGHT?                  */
    //write them all to the file
    // write ibit to file
    bio_write(1,(void*) ibit);
    // write dbit to file
    bio_write(2,(void*) dbit);
    // write root inode to file
    writei(0,rootI);
    
	return 0;
}


/* 
 * FUSE file operations
 */
static void *tfs_init(struct fuse_conn_info *conn) {

	// Step 1a: If disk file is not found, call mkfs
    if( dev_open(diskfile_path) == -1 ){
        puts("FIRST TIME WE RUN INIT");
        tfs_mkfs();
    }else{
        // Step 1b: If disk file is found, just initialize in-memory data structures
        // and read superblock from disk
        char* buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
        bio_read(0,(void*) buffer);
        sb = (struct superblock*) buffer;
    }

	return NULL;
}

static void tfs_destroy(void *userdata) {

	// Step 1: De-allocate in-memory data structures
    // de allocate the super block.
    free(sb);

	// Step 2: Close diskfile
    dev_close();
}

static int tfs_getattr(const char *path, struct stat *stbuf) {

    puts("I AM IN GETATTR!!!!!!!!!!!!!!!!!!");

	// Step 1: call get_node_by_path() to get inode from path
    struct inode* inode = (struct inode*) malloc(sizeof(struct inode));
    // assume all paths start at root?                                            *****
    // also have to check if it even finds it
    int getRes = 0;
    getRes = get_node_by_path(path, 0, inode);
    if( getRes == -1 ){
        // error
        printf("Could not resolve path\n");
        return -ENOENT;
    }
    
	// Step 2: fill attribute of file into stbuf from inode
    // memset the stat
    memset(stbuf,0,sizeof(struct stat));
    // set everything
    stbuf->st_nlink  = inode->link;
    stbuf->st_atime = (inode->vstat).st_atime;
    stbuf->st_mtime = (inode->vstat).st_mtime;
    stbuf->st_uid = (inode->vstat).st_uid;
    stbuf->st_gid = (inode->vstat).st_gid;
    stbuf->st_ino = inode->ino;
    stbuf->st_size = inode->size;
    stbuf->st_mode = (inode->vstat).st_mode;
    return 0;
}

static int tfs_opendir(const char *path, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path
    struct inode* inode = (struct inode*) malloc(sizeof(struct inode));
    // since we are only dealing with absolute paths we can just pass ino 0
    int getRes = get_node_by_path(path, 0, inode);

    puts("I AM IN OPENDIR!!!!!!!!!!!!!!!!!!!!!!!!!!");

	// Step 2: If not find, return -1
    if( getRes == -1 ){
        puts("Dir does not exist");
        return -1;
    }

    // update atime
    time( &((inode->vstat).st_atime) );

    return 0;
}

static int tfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	puts("I AM IN READDIR!!!!!!!!!!!!!");
    
    // Step 1: Call get_node_by_path() to get inode from path
    struct inode* inode = (struct inode*) malloc(sizeof(struct inode));
    // since we are only dealing with absolute paths we can just pass ino 0
    int getRes = get_node_by_path( path, 0, inode );
    if( getRes == -1 ){
        puts("Dir does not exist");
        return -ENOENT;
    }

	// Step 2: Read directory entries from its data blocks, and copy them to filler
    int directPtrIdx = 0;
    int blockIdx = 0;
    // loop through all of the valid blocks in the inode
    for( directPtrIdx = 0; directPtrIdx < (sizeof(inode->direct_ptr)/sizeof(int)); directPtrIdx++ ){
        // if it actually points to something
        if( (inode->direct_ptr)[directPtrIdx] >= 0 ){
            // need a buffer to put the block of data we are reading
            char* dataBuffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
            int blockNum = (inode->direct_ptr)[directPtrIdx] + sb->d_start_blk;
            bio_read( blockNum, (void*) dataBuffer );

            // loop through all of the dirents inside this block
            for( blockIdx = 0; blockIdx < direntsPerBlock; blockIdx++){
                // we want to only copy the bytes of a dirent into this buffer
                char* dbuff = (char*) malloc(sizeof(char) * sizeof(struct dirent));
                int direntIdx = 0;
                // copy byte by byte to our dirent buffer
                for(direntIdx = 0; direntIdx < sizeof(struct dirent); direntIdx++){
                    dbuff[direntIdx] = dataBuffer[direntIdx + blockIdx * sizeof(struct dirent)];
                }

                // check if the dirent is valid
                if( ((struct dirent*) dbuff)->valid == 1 ){
                    // call filler on this because it is valid
                    // make sure filler works
                    if( filler(buffer, ((struct dirent*) dbuff)->name, NULL, 0) != 0 ){
                        // eror with filler throuhg erorr
                        puts("Error with filler");
                        return -ENOMEM;
                    }
                }       
            }
        }
    }
    // success
	return 0;
}


static int tfs_mkdir(const char *path, mode_t mode) {
    // create copies of the path
    char* dirc = strdup(path);
    char* basec = strdup(path);

    puts("I AM IN MKDIR!!!!!!!!!!!!!!!!!!!!!!!!");

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name
    char* dirName = dirname(dirc);
    char* target = basename(basec);

	// Step 2: Call get_node_by_path() to get inode of parent directory
    struct inode* dirInode = (struct inode*) malloc(sizeof(struct inode));
    // since we are only using absolute paths we can pass it roots inode number
    int getRes = get_node_by_path(dirName, 0, dirInode);
    if( getRes == -1 ){
        puts("Directory does not exist");
        return -ENOENT; 
    }

	// Step 3: Call get_avail_ino() to get an available inode number
    int newIno = get_avail_ino();

	// Step 4: Call dir_add() to add directory entry of target directory to parent directory
    int addRes = dir_add( *dirInode, newIno, target, strlen(target) );
    if( addRes == -1 ){
        printf("Error adding!!!!!!!!!!!!!!!!");
        // could be because file already exists or do data
        // return -ENOMEM;                                                          *****
        return -EEXIST;
    }

	// Step 5: Update inode for target directory
    struct inode* targetInode = (struct inode*) malloc(sizeof(struct inode));
    targetInode->ino = newIno;
    targetInode->valid = 1;
    targetInode->size = 0;
    targetInode->type = 1;
    targetInode->link = 1;
    /* loop through all values and set them to -1 */
    int counter = 0;
    for(counter = 0; counter < (sizeof(targetInode->direct_ptr) / sizeof(int)); counter++){
        (targetInode->direct_ptr)[counter] = -1;
    }
    for(counter = 0; counter < (sizeof(targetInode->indirect_ptr) / sizeof(int)); counter++){
        (targetInode->indirect_ptr)[counter] = -1;
    }
    /* set the vstat values */
    (targetInode->vstat).st_ino = newIno;
    //(targetInode->vstat).st_mode = S_IFDIR | 0755;
    (targetInode->vstat).st_mode = S_IFDIR | mode;
    (targetInode->vstat).st_uid = getuid();
    (targetInode->vstat).st_gid = getgid();
    (targetInode->vstat).st_nlink = 1;
    (targetInode->vstat).st_size = 0;
    time( &((targetInode->vstat).st_atime) );

	// Step 6: Call writei() to write inode to disk
    writei(newIno, targetInode);


	return 0;
}

static int tfs_rmdir(const char *path) {

    puts("I AM IN RMDIR!!!!!!!!!!!!!!!");

    // Step 0: get the bitmaps
    // get inode bitmap
    char* buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
    char* ibuff = (char*) malloc( sizeof(char) * (MAX_INUM / 8));
    bio_read(1, (void*) buffer);
    // copy first bytes to ibuff
    int counter = 0;
    for(counter = 0; counter < MAX_INUM / 8; counter++){
        ibuff[counter] = buffer[counter];
    }
    // cast bitmap
    bitmap_t ibit = (bitmap_t) ibuff;
    
    // reset buffer
    memset(buffer, 0, BLOCK_SIZE);
    // get data block bitmap
    char* dbuff = (char*) malloc(sizeof(char) * (MAX_DNUM / 8));
    bio_read(2,(void*) buffer);
    // copy first bytes to dbuff
    for(counter = 0; counter < MAX_DNUM / 8; counter++){
        dbuff[counter] = buffer[counter];
    }
    // cast bitmap
    bitmap_t dbit = (bitmap_t) dbuff;
    

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name
    // create copies of the path
    char* fpath = strdup(path);
    char* dirc = strdup(path);
    char* basec = strdup(path);
    char* dirName = dirname(dirc);
    char* target = basename(basec);

	// Step 2: Call get_node_by_path() to get inode of target directory
    struct inode* targetInode = (struct inode*) malloc(sizeof(struct inode));
    // since we are only using absolute paths we can pass it roots inode number
    int getRes = get_node_by_path(fpath, 0, targetInode);
    if( getRes == -1 ){
        puts("Directory does not exist");
        return -ENOENT; 
    }

	// Step 3: Clear data block bitmap of target directory
    // have to loop through all f the direct pointers and reclaim them in the bitmap
    int directPtrIdx = 0;
    for( directPtrIdx = 0; directPtrIdx < (sizeof(targetInode->direct_ptr)/sizeof(int)); directPtrIdx++){
        // check if whether or not the direct pointer is set
        if( (targetInode->direct_ptr)[directPtrIdx] >= 0 ){
            // reclaim the data block in bitmap
            unset_bitmap( dbit, (targetInode->direct_ptr)[directPtrIdx] );
            // set val to -1 bc we reclaimed it
            (targetInode->direct_ptr)[directPtrIdx] = -1;        
        }
    }
    // write the bitmap back to disk
    bio_write(2,(void*) dbit);


	// Step 4: Clear inode bitmap and its data block
    unset_bitmap(ibit, targetInode->ino);
    // write the bitmap back to disk
    bio_write(1,(void*) ibit);
    /*                  What does it mean by its data block?                    */
    /*          Do we have to update the target inode and write it to the block in inode table?        */


	// Step 5: Call get_node_by_path() to get inode of parent directory
    struct inode* dirInode = (struct inode*) malloc(sizeof(struct inode));
    // since we are only using absolute paths we can pass it roots inode number
    getRes = get_node_by_path(dirName, 0, dirInode);
    if( getRes == -1){
        puts("Directory does not exist");
        return -ENOENT;
    }

	// Step 6: Call dir_remove() to remove directory entry of target directory in its parent directory
    int removeRes = dir_remove( *dirInode, target, strlen(target) );
    if( removeRes == -1 ){
        puts("Unable to remove");
        return -ENOENT;
    }
    // write the update inode?
    writei(dirInode->ino, dirInode);

	return 0;
}

static int tfs_releasedir(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

    puts("I AM IN CREAT!!!!!!!!!!!!!!!!!!");

	// Step 1: Use dirname() and basename() to separate parent directory path and target file name

	// Step 2: Call get_node_by_path() to get inode of parent directory

	// Step 3: Call get_avail_ino() to get an available inode number

	// Step 4: Call dir_add() to add directory entry of target file to parent directory

	// Step 5: Update inode for target file

	// Step 6: Call writei() to write inode to disk


    // update atime


/*
    int retstat = 0;
    char fpath[PATH_MAX];
    tfs_fullpath(fpath,path);
    
    if(S_ISREG(mode)){
        open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
        retstat = -errno;
    }else
        if(S_ISFIFO(mode)){
            if(retstat >= 0){
                close(retstat);
                retstat = -errno;
            }
        }else{
            if(S_ISFIFO(mode)){
                mkfifo(fpath,mode);
                retstat = -errno;
            }
        }
*/
	return 0;
}

static int tfs_open(const char *path, struct fuse_file_info *fi) {

    puts("I AM IN OPEN!!!!!!!!!!!!!!!!!!!");

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: If not find, return -1

    // update atime
	return 0;
}

static int tfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {

    puts("I AM IN READ!!!!!!!!!!!!!!!!!!!!");

	// Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: copy the correct amount of data from offset to buffer

	// Note: this function should return the amount of bytes you copied to buffer
	return 0;
}

static int tfs_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	
    puts("I AM IN WRITE!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    
    // Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: Write the correct amount of data from offset to disk

	// Step 4: Update the inode info and write it to disk

	// Note: this function should return the amount of bytes you write to disk
	
    // update mtime

    return size;
}

static int tfs_unlink(const char *path) {

    puts("I AM IN UNLINK!!!!!!!!!!!!!!!!!!!!!!!!");

	// Step 1: Use dirname() and basename() to separate parent directory path and target file name

	// Step 2: Call get_node_by_path() to get inode of target file

	// Step 3: Clear data block bitmap of target file

	// Step 4: Clear inode bitmap and its data block

	// Step 5: Call get_node_by_path() to get inode of parent directory

	// Step 6: Call dir_remove() to remove directory entry of target file in its parent directory

	return 0;
}

static int tfs_truncate(const char *path, off_t size) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_release(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
	return 0;
}

static int tfs_flush(const char * path, struct fuse_file_info * fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_utimens(const char *path, const struct timespec tv[2]) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}


static struct fuse_operations tfs_ope = {
	.init		= tfs_init,
	.destroy	= tfs_destroy,

	.getattr	= tfs_getattr,
	.readdir	= tfs_readdir,
	.opendir	= tfs_opendir,
	.releasedir	= tfs_releasedir,
	.mkdir		= tfs_mkdir,
	.rmdir		= tfs_rmdir,

	.create		= tfs_create,
	.open		= tfs_open,
	.read 		= tfs_read,
	.write		= tfs_write,
	.unlink		= tfs_unlink,

	.truncate   = tfs_truncate,
	.flush      = tfs_flush,
	.utimens    = tfs_utimens,
	.release	= tfs_release
};


int main(int argc, char *argv[]) {
	int fuse_stat;

    printf("THE SIZE OF AN INODE IS: %lu\n", sizeof(struct inode));
    printf("THE SIZE OF A DIRENT IS: %lu\n", sizeof(struct dirent));
    printf("THE SIEZ OF A SUPERBLOCK IS: %lu\n",sizeof(struct superblock));
	
    getcwd(diskfile_path, PATH_MAX);
	strcat(diskfile_path, "/DISKFILE");

	fuse_stat = fuse_main(argc, argv, &tfs_ope, NULL);

	return fuse_stat;
}

