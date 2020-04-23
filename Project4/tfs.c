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

/*
 * Puts full path into buffer

void tfs_fullpath(char fpath[PATH_MAX], const char* path){
    
    strcpy(fpath,"/ilab/users/ajb393/416/Project4/rootdir");
    //getcwd(fpath,PATH_MAX);
    strncat(fpath,path,PATH_MAX);
    printf("fullpath(): %s\n",fpath);
}
*/


// Declare your in-memory data structures here

/* 
 * Get available inode number from bitmap
 */
int get_avail_ino() {

	// Step 1: Read inode bitmap from disk
	
	// Step 2: Traverse inode bitmap to find an available slot

	// Step 3: Update inode bitmap and write to disk 

	return 0;
}

/* 
 * Get available data block number from bitmap
 */
int get_avail_blkno() {

	// Step 1: Read data block bitmap from disk
	
	// Step 2: Traverse data block bitmap to find an available slot

	// Step 3: Update data block bitmap and write to disk 

	return 0;
}

/* 
 * inode operations
 */
int readi(uint16_t ino, struct inode *inode) {

  // Step 1: Get the inode's on-disk block number

  // Step 2: Get offset of the inode in the inode on-disk block

  // Step 3: Read the block from disk and then copy into inode structure

	return 0;
}

int writei(uint16_t ino, struct inode *inode) {

	// Step 1: Get the block number where this inode resides on disk
	
	// Step 2: Get the offset in the block where this inode resides on disk

	// Step 3: Write inode to disk 

	return 0;
}


/* 
 * directory operations
 */
int dir_find(uint16_t ino, const char *fname, size_t name_len, struct dirent *dirent) {

  // Step 1: Call readi() to get the inode using ino (inode number of current directory)

  // Step 2: Get data block of current directory from inode

  // Step 3: Read directory's data block and check each directory entry.
  //If the name matches, then copy directory entry to dirent structure

	return 0;
}

int dir_add(struct inode dir_inode, uint16_t f_ino, const char *fname, size_t name_len) {

	// Step 1: Read dir_inode's data block and check each directory entry of dir_inode
	
	// Step 2: Check if fname (directory name) is already used in other entries

	// Step 3: Add directory entry in dir_inode's data block and write to disk

	// Allocate a new data block for this directory if it does not exist

	// Update directory inode

	// Write directory entry

	return 0;
}

int dir_remove(struct inode dir_inode, const char *fname, size_t name_len) {

	// Step 1: Read dir_inode's data block and checks each directory entry of dir_inode
	
	// Step 2: Check if fname exist

	// Step 3: If exist, then remove it from dir_inode's data block and write to disk

	return 0;
}

/* 
 * namei operation
 */
int get_node_by_path(const char *path, uint16_t ino, struct inode *inode) {
	
	// Step 1: Resolve the path name, walk through path, and finally, find its inode.
	// Note: You could either implement it in a iterative way or recursive way

	return 0;
}

/* 
 * Make file system
 */
int tfs_mkfs() {

	// Call dev_init() to initialize (Create) Diskfile
    dev_init(diskfile_path);

	// write superblock information
    struct superblock* sb = (struct superblock*) malloc( sizeof(struct superblock) );
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


	// initialize data block bitmap


	// update bitmap information for root directory


	// update inode for root directory


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
        puts("NOT FIRST TIME WE RUN IT");
        char buffer[BLOCK_SIZE];
        bio_read(0,(void*) buffer);
        struct superblock* sb = (struct superblock*) buffer;
        printf("max_inum: %u, max_dnum: %u\n",(unsigned int) sb->max_inum,(unsigned int) sb->max_dnum);
        
    }


  // Step 1b: If disk file is found, just initialize in-memory data structures
  // and read superblock from disk

	return NULL;
}

static void tfs_destroy(void *userdata) {

	// Step 1: De-allocate in-memory data structures

	// Step 2: Close diskfile

}

static int tfs_getattr(const char *path, struct stat *stbuf) {

	// Step 1: call get_node_by_path() to get inode from path

	// Step 2: fill attribute of file into stbuf from inode
		
        
        stbuf->st_mode   = S_IFDIR | 0755;
		stbuf->st_nlink  = 2;
		time(&stbuf->st_mtime);
        
/*
  
    int retstat;
    char fpath[PATH_MAX];
    tfs_fullpath(fpath,path);
    retstat = lstat(fpath,stbuf);
    printf("getattr(): resttat: %d\n",retstat);
    if(retstat == 0)
        return 0;
    printf("getattr(): error is %s\n",strerror(errno));
    retstat = -1*errno;
*/
  	return 0;
}

static int tfs_opendir(const char *path, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: If not find, return -1




/*
    DIR* dp;
    int retstat = 0;
    char fpath[PATH_MAX];

    tfs_fullpath(fpath,path);
    dp = opendir(fpath);
    if(dp == NULL)
        retstat = -errno;

    fi->fh = (intptr_t) dp;
*/
    return 0;
}

static int tfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: Read directory entries from its data blocks, and copy them to filler


/*
    int retstat = 0;
    DIR* dp;
    struct dirent* de;

    dp = (DIR*) (uintptr_t) fi->fh;

    // set errno to 0?
    errno = 0;
    de = readdir(dp);
    if(de == 0){
        retstat = -errno;
        return retstat;
    }
    
    do{
        if(filler(buffer,de->d_name,NULL,0) != 0){
            return -ENOMEM;
        }
    }while( (de = readdir(dp)) != NULL);
*/
	return 0;
}


static int tfs_mkdir(const char *path, mode_t mode) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name

	// Step 2: Call get_node_by_path() to get inode of parent directory

	// Step 3: Call get_avail_ino() to get an available inode number

	// Step 4: Call dir_add() to add directory entry of target directory to parent directory

	// Step 5: Update inode for target directory

	// Step 6: Call writei() to write inode to disk


/*
    char fpath[PATH_MAX];
    tfs_fullpath(fpath,path);
    printf("Trying to makedir: %s\n",fpath);

	//mkdir(fpath,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(fpath,mode);
*/
	return 0;
}

static int tfs_rmdir(const char *path) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name

	// Step 2: Call get_node_by_path() to get inode of target directory

	// Step 3: Clear data block bitmap of target directory

	// Step 4: Clear inode bitmap and its data block

	// Step 5: Call get_node_by_path() to get inode of parent directory

	// Step 6: Call dir_remove() to remove directory entry of target directory in its parent directory



/*
    char fpath[PATH_MAX];
    tfs_fullpath(fpath,path);
    printf("Trying to rmdir: %s\n",fpath);
    rmdir(fpath);
*/
	return 0;
}

static int tfs_releasedir(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target file name

	// Step 2: Call get_node_by_path() to get inode of parent directory

	// Step 3: Call get_avail_ino() to get an available inode number

	// Step 4: Call dir_add() to add directory entry of target file to parent directory

	// Step 5: Update inode for target file

	// Step 6: Call writei() to write inode to disk




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

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: If not find, return -1

	return 0;
}

static int tfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {

	// Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: copy the correct amount of data from offset to buffer

	// Note: this function should return the amount of bytes you copied to buffer
	return 0;
}

static int tfs_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	// Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: Write the correct amount of data from offset to disk

	// Step 4: Update the inode info and write it to disk

	// Note: this function should return the amount of bytes you write to disk
	return size;
}

static int tfs_unlink(const char *path) {

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

