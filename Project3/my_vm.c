#include "my_vm.h"

/*                      List of Globals                     */
// Bool for if it has run at least once
int initialized = 0;

// Physical Mem
char* physicalMem;

// Bit arrays
char* virtBitArr;
char* physBitArr;

// how many bits for the page dir
int numPageDirBits;
// how many bits for the page table
int numPageTableBits;
// how many bits for the offset
int numOffBits;
// num page dir entries
int numPageDirEntries;
// num page table entries
int numPageTableEntries;
// how many virtual entries
int numVirtEntries;
// how many physical entries
int numPhysEntries;

// pointer to our page Directory
pde_t* pageDir;

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating

    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
    
    // calculate nump bits for everything
    int p = PGSIZE;
    numOffBits = 0;
    while (p > 1) {
        p>>=1;
        numOffBits++;
    }
    int numVPNBits = 32 - numOffBits;
    numPageDirBits = numVPNBits / 2;
    numPageTableBits = 32 - numOffBits - numPageDirBits;

    // calcuate how many entries in a page directory (2^numBits)
    numPageDirEntries = 1<<numPageDirBits;
    
    // Calculate how many entries in an inner page tbale (2^numBits)
    numPageTableEntries = 1 << numPageTableBits;

    // calculate the num of total virtual pages
    numVirtEntries = MAX_MEMSIZE / PGSIZE;
    //numVirtEntries = numPageDirEntries * numPageTableEntries;
    
    // calculate the num of total physical pages
    numPhysEntries = MEMSIZE / PGSIZE;

    // need to allocate the physical memory here (based off of MEMSIZE)
    physicalMem = (char*) malloc( sizeof(char) * MEMSIZE );

    // need to allocate the bitarrays (this is based off of number of entries)
    // We divide by 8 because each char can fit 8 bits (and thus 8 entries)
    virtBitArr = (char*) malloc( numVirtEntries / 8 );
    physBitArr = (char*) malloc( numPhysEntries / 8 );
    // initialize them all as 0
    memset(virtBitArr,0,numVirtEntries / 8);
    memset(physBitArr,0,numPhysEntries / 8);

    /*                  TESTING                 */
    //int counter = 0;
    //for(counter = 0; counter < numPhysEntries;counter++){
    //    setBit(physBitArr,counter);
    //}
    /*                  TESTING                 */

    // set the 0th bit in the virtBitArr to 1 (we never want to use this one)
    // because the address would be 0x00000000 which should be what we say is NULL
    setBit(virtBitArr,0);

    // Allocate page directory
    pageDir = (pde_t*) malloc( numPageDirEntries * sizeof( pde_t ) );
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */

}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */


    //If translation not successfull
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    // cast these values back
    unsigned int* vir_pg_index = (unsigned int*) va;
    unsigned long* phys_pg_addr = (unsigned long*) pa;

    //int pgdir_ind = ((*vir_pg_index)/num_pg_dir_entries);


    return 0;

}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
    // va should be an unsigned long
    unsigned long va;

    // pageNum in bit arr we are giving back to them
    unsigned int startIndex = 0;
    // flag to see if mem is contig
    int contig = 0;

    // Use virtual address bitmap to find the next free contiguous pages
    int counter = 0;
    // loop through all of the virtual entries (except the first one)
    for( counter = 1; counter < numVirtEntries; counter++ ){
        // check if curr bit is not in use
        if( testBit( virtBitArr, counter ) == 0 ){
            // set it to in use
            setBit( virtBitArr, counter );
            // check if we are already in contig
            if( contig == 0 ){
                // we are not, So this would be our start index
                startIndex = counter;
                // mark we are now contig
                contig = 1;
            }else{
                // do nothin bc we are just finding the bits after start index
            }
        }else{
            // the bit is in use
            // check if we have marked any bits already
            if( contig == 1 ){
                // need to reset all of the bits that have been set so far
                int x = 0;
                for( x = startIndex; x < counter; x++ ){
                    clearBit( virtBitArr, x );
                }
            }
            // reset start index
            startIndex = 0;
            // set flag to 0 bc we are starting over
            contig = 0;
        }
        // check if I have all contig pages
        if( contig == 1 && (counter - startIndex + 1 == num_pages) ){
            break;
        }
    }

    // check if we found the pages
    if( startIndex == 0 ){
        printf("Error finding virtual pages\n");
        return NULL;
    }

    // calculate the va from the first VPN
    // find the index in the page dir
    unsigned int outerPageIndex = startIndex / numPageTableEntries;
    // find the index in the inner page table
    unsigned int innerPageIndex = startIndex % numPageTableEntries;
    // assume offset is 0
    
    // build the va
    va |= outerPageIndex << (numPageTableBits + numOffBits);
    va |= innerPageIndex << numOffBits;
    
    return (void*) va;

}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *a_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    // This is only ran the first time a_malloc() is called
    // Initialize all of the variables, page dir, physical mem, and bit arrs
    if( initialized == 0 ){
        initialized = 1;
        set_physical_mem();
    }

    // calculate the number of pages needed
    int numPagesNeeded = num_bytes / PGSIZE;
    // check if we need another page
    if( num_bytes % PGSIZE != 0)
        numPagesNeeded += 1;

    // find all of the contiguous virtual pages (this is an array of all the virtual pages)
    unsigned long* va =  get_next_avail( numPagesNeeded );

    // if va is NULL that means that it failed
    if( va == NULL ){
        printf("Error getting Virtual Pages\n");
        return NULL;
    }

    // Find all of the physical pages ( this is an array of all the physical pages)
    unsigned long* pa = get_next_avail_phys( numPagesNeeded );

    // if first element of pa is 0 that means that there are not enough phys pages
    if( ((unsigned long*)pa)[0] == 0 ){
        printf("Error getting physical Pages\n");
        // need to set the bits corresponding to virt pages back to 0
        resetVirtBits(va,numPagesNeeded);
        return NULL;
    }

    // Loop through all physical pages mapping virtual pages to the physical address
    int counter = 0;
    for( counter = 0; counter < numPagesNeeded; counter++ ){
        //int retVal = page_map( pageDir, (void*)va[counter],(void*) pa[counter] );
    }
    
    // return the virtual addres of the first virtual page
    return (void*) va;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void a_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
     
    
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */




}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */




}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */

       
}

/*                  Private Functions                   */

/*
 * Set these virtual bits back to 0 because some error so we
 * are not using them anymore.
*/ 
void resetVirtBits(void* va,int numPagesNeeded){
    // cast it to the unsigned long so we can manipulate it
    unsigned long virtAddr = (unsigned long) va;
    
    // calculate the outer page directory index
    unsigned long pageDirIndex = getTopIndex( virtAddr );

    // calculate the inner page directory index
    unsigned long pageTableIndex = getMidIndex( virtAddr );

    // calculate the VPN
    unsigned long vpn = pageDirIndex * numPageTableEntries + pageTableIndex;

    // loop through all the VPNs and clear the bit
    int counter = 0;
    for( counter = 0; counter < numPagesNeeded; counter++ ){
        //printf("bit %d: %d\n",vpn + counter,testBit(virtBitArr,vpn + counter));
        clearBit( virtBitArr, vpn + counter);
        //printf("bit %d: %d\n",vpn + counter,testBit(virtBitArr,vpn + counter));
    }
}

/*
 * Find and return an array of all the physical pages
 * that I will be mapping into the virtual ones
*/
void* get_next_avail_phys(int numNeededPages){
    // initialize the array
    unsigned long* pa = (unsigned long*) malloc( numNeededPages * sizeof(unsigned long) );
    int x = 0;
    int y = 0;
    for( x = 0; x < numNeededPages; x++ ){
        pa[x] = 0;
    }
    int errorPage = 0;


    // traverse through the physical bit arr to look for num Needed Pages
    for( x = 0; x < numNeededPages; x++ ){
        for( y = 0; y < numPhysEntries; y++){
            // check if the curr bit/page is not in use
            if( testBit( physBitArr, y ) == 0 ){
                // set the bit as in use
                setBit( physBitArr, y );
                // calculate the phys address to put into array
                pa[x] = ((unsigned long) &physicalMem) + (y * PGSIZE);
                // break bc we found an address
                break;
            }
        }
        // check if we found one or not
        if( pa[x] == 0 ){
            errorPage = 1;
            break;
        }
    }

    // do something if there was a failure
    if( errorPage == 1 ){
        printf("Error finding physical Page\n");
        // we need to go back to all of the physical pages in the arr and switch them back
        for( x = 0; x < numNeededPages; x++ ){
            if( pa[x] != 0 ){
                // calculate the PPN
                unsigned long ppn = pa[x] - ((unsigned long) &physicalMem);
                ppn /= PGSIZE;
                // set the bit back to 0
                clearBit( physBitArr, ppn );
                // set val in array to 0
                pa[x] = 0;
            }
        }
    }

    return (void*) pa;
}

/*
 * Set the kth bit to 1 inside the passed bit array
*/
void setBit(char* bitArr, int k){
    bitArr[k/(sizeof(char)*8)] |= 1 << (k%(sizeof(char)*8));
}

/*
 * clear (set to 0) the kth bit inside the passed bit array
*/
void clearBit(char* bitArr, int k){
    bitArr[k/(sizeof(char)*8)] &= ~(1 << (k%(sizeof(char)*8)));
}

/*
 * return the value at the kth bit inside the passed bit array
*/
int testBit(char* bitArr, int k){
    return ( (bitArr[k/(sizeof(char)*8)] & (1 << (k%(sizeof(char)*8)) )) != 0 );
}

/*
 * Prints the binary of a number
*/
void printBits(unsigned long val){
    char buffer[100];
    memset(buffer,0, 100);
    unsigned long num = val;
    int numChars = 0;
    while (num >>= 1) numChars++;
    int counter = numChars;
    do{
        if( val % 2 == 0)
            buffer[counter] = '0';
        else
            buffer[counter] = '1';
        val /= 2;
        counter -= 1;
    }while( val > 0 );
    printf("%s\n",buffer);
}

/*
 * Gets the page directory index
*/
unsigned long getTopIndex(unsigned long va){
    return (va >> (32 - numPageDirBits));
}

/*
 * Gets the page table index
*/
unsigned long getMidIndex(unsigned long va){
    unsigned long mid_bits_value = 0;

    va = va >> numOffBits;

    unsigned long outer_bits_mask = (1 << numPageTableBits);

    outer_bits_mask = outer_bits_mask-1;

    mid_bits_value = va & outer_bits_mask;

    return mid_bits_value;

}
