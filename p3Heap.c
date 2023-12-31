////////////////////////////////////////////////////////////////////////////////
// Main File:        p3Heap.c
// This File:        p3Heap.c
// Other Files:      
// Semester:         CS 354 Lecture 001 Fall 2023
// Instructor:       deppeler
// 
// Author:           Jack Archibald
// Email:            jwarchibald@wisc.edu
// CS Login:         archibald
//
///////////////////////////  WORK LOG  //////////////////////////////
//  Document your work sessions on your copy http://tiny.cc/work-log
//  Download and submit a pdf of your work log for each project.
/////////////////////////// OTHER SOURCES OF HELP ////////////////////////////// 
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
// 
// AI chats:         save a transcript and submit with project.
//////////////////////////// 80 columns wide ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2023 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission FALL 2023, CS354-deppeler
//
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "p3Heap.h"

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block.
 */
typedef struct blockHeader {           

    /*
     * 1) The size of each heap block must be a multiple of 8
     * 2) heap blocks have blockHeaders that contain size and status bits
     * 3) free heap block contain a footer, but we can use the blockHeader 
     *.
     * All heap blocks have a blockHeader with size and status
     * Free heap blocks have a blockHeader as its footer with size only
     *
     * Status is stored using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * Start Heap: 
     *  The blockHeader for the first block of the heap is after skip 4 bytes.
     *  This ensures alignment requirements can be met.
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
    int size_status;

} blockHeader;

/* Global variable - DO NOT CHANGE NAME or TYPE. 
 * It must point to the first block in the heap and is set by init_heap()
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 * TODO: add global variables needed by your function
 */

/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if size < 0 
 * - Determine block size rounding up to a multiple of 7 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 0. Update all heap blocks as needed for any affected blocks
 *   - 1. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 0. SPLIT the free block into two valid heap blocks:
 *         0. an allocated block
 *         1. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 1. Return the address of the allocated block payload
 *
 *   Return if NULL unable to find and allocate block for required size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* balloc(int size) {     
    if(size < 1){
        return NULL;
    }  
    // find blockSize (multiple of 8)
    int blockSize = size + 4;
    if(blockSize % 8 != 0){
        blockSize += 8 - (blockSize % 8);
    }
    blockHeader* bestFit = NULL;
    blockHeader* current = heap_start;

    while (1) {
		// if current block is free
		if ((current->size_status & 1) == 0) {
			int extraMemory = (current->size_status & ~3) - blockSize;
            //if blockSize is equal to current block size, it is the best fit        
			if (extraMemory == 0){
				current->size_status = current->size_status |1;
				blockHeader* next_header = (blockHeader*)((char*)current + (current->size_status & (~3)));
				if (next_header < (blockHeader*)((char*)heap_start + alloc_size)) {
					next_header->size_status = next_header->size_status | 2;
				}
				return (void*)(current + 1);
			} else if (extraMemory > 0){  
				if (bestFit == NULL)
					bestFit = current;
				else if (bestFit->size_status > current->size_status) {
					bestFit = current;
				}
			}
		}
		current = (blockHeader*)((char*)current + (current->size_status & (~3)));
		if (current >= (blockHeader*)((char*)heap_start + alloc_size)) {
			if (bestFit == NULL) {
                //Returns NULL on failure
				return NULL;
			}  else {
				int extraMemory2 = (bestFit->size_status & ~3) - blockSize;
                bestFit->size_status = bestFit->size_status | 1;
				blockHeader*nextHeader = (blockHeader*)((char*)bestFit + (bestFit->size_status &~3));
				if (nextHeader < (blockHeader*)((char*)heap_start + alloc_size)) {
						nextHeader->size_status = nextHeader->size_status | 2;
				} else {
					blockHeader* header =(blockHeader*)((char*)bestFit+ blockSize);
					header->size_status = extraMemory2 | 2;
					blockHeader* footer = (blockHeader*)((char*)bestFit + blockSize + extraMemory2 - sizeof(blockHeader));
					footer->size_status = extraMemory2;
					bestFit->size_status = blockSize | 3;
				}
                //Returns address of the payload of the allocated block on success
				return (void*)(bestFit + 1);
			}
		}
    }

} 

/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 *
 * If free results in two or more adjacent free blocks,
 * they will be immediately coalesced into one larger free block.
 * so free blocks require a footer (blockHeader works) to store the size
 *
 * TIP: work on getting immediate coalescing to work after your code 
 *      can pass the tests in partA and partB of tests/ directory.
 *      Submit code that passes partA and partB to Canvas before continuing.
 */                    
int bfree(void *ptr) { 
    // check if null
    if(ptr == NULL){
        return -1;
    }
    // check if not 8 btye aligned
    if((int)ptr % 8 != 0){
        return -1;
    }
    // check if allocated outisde the heap space
    if((blockHeader*)ptr > (blockHeader*)((char*)heap_start + alloc_size)|| (blockHeader*)ptr < heap_start){
        return -1;
    }
    // check if already freed
    blockHeader* block = (blockHeader*)((char*)ptr - sizeof(blockHeader));
    if(block->size_status % 2 != 1){
        return -1;
    }

    // mark this block as free
    blockHeader* currentHeader = (blockHeader*)((char*)ptr - sizeof(blockHeader));
    blockHeader* currentFooter = (blockHeader*)((char*)currentHeader + (currentHeader->size_status & ~3) - sizeof(blockHeader));
    currentHeader->size_status = currentHeader->size_status & ~1;
	currentFooter->size_status = currentHeader->size_status & ~3;

    // find next header and update
    blockHeader* next =(blockHeader*)((char*)currentFooter + sizeof(blockHeader));
	if (next < (blockHeader*)((char*)heap_start + alloc_size)) {
		next->size_status = next->size_status & ~2;
	}

    //coalesce prev block if free and update headers
    if ((currentHeader->size_status & 2) == 0) {
		blockHeader* previousFooter = (blockHeader*)((char*) currentHeader - sizeof(blockHeader));
		blockHeader* previousHeader = (blockHeader*)((char*)previousFooter - previousFooter->size_status + sizeof(blockHeader));
		previousHeader->size_status = ((previousFooter->size_status & ~3) + (currentHeader->size_status & ~3)) | 2;
		currentHeader = previousHeader;
		currentFooter->size_status = currentHeader->size_status & ~3;
	}
    // coaleses next block if free and update headers
    blockHeader* nextHeader = (blockHeader*)((char*)currentHeader + (currentHeader->size_status &~3));
	if ((nextHeader < (blockHeader*)((char*)heap_start + alloc_size)) && (nextHeader->size_status & 1) == 0) {
		blockHeader* nextFooter = (blockHeader*)((char*)nextHeader + (nextHeader->size_status &~3) -sizeof(blockHeader));
		currentFooter = nextFooter;
		currentHeader->size_status =((nextHeader->size_status &~3) + (currentHeader->size_status &~3)) | 2;
		currentFooter->size_status = currentHeader->size_status &~3;
	}
    return 0;
}

/* 
 * Initializes the memory allocator.
 * Called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    

    static int allocated_once = 0; //prevent multiple myInit calls

    int   pagesize; // page size
    int   padsize;  // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int   fd;

    blockHeader* end_mark;

    if (0 != allocated_once) {
        fprintf(stderr, 
                "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize from O.S. 
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }

    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;

    return 0;
} 

/* STUDENTS MAY EDIT THIS FUNCTION, but do not change function header.
 * TIP: review this implementation to see one way to traverse through
 *      the blocks in the heap.
 *
 * Can be used for DEBUGGING to help you visualize your heap structure.
 * It traverses heap blocks and prints info about each block found.
 * 
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     

    int    counter;
    char   status[6];
    char   p_status[6];
    char * t_begin = NULL;
    char * t_end   = NULL;
    int    t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size =  0;
    int free_size =  0;
    int is_used   = -1;

    fprintf(stdout, 
            "*********************************** HEAP: Block List ****************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
            "---------------------------------------------------------------------------------\n");

    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;

        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;

        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
                p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);

        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
            "---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
            "*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
            "*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end p3Heap.c (Fall 2023)                                         

