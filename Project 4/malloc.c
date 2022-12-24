#include<stdlib.h>
#include<stdio.h>
#include <stdint.h>
#include <string.h>
#include "mymalloc.h"


/* Basic constants and macros: */
#define WSIZE      8 /* Word and header/footer size (bytes) */
#define DSIZE      16    /* Doubleword size (bytes) */


/* Pack a size and allocated bit into a word. */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p. */
#define GET(p)       (*(uintptr_t *)(p))
#define PUT(p, val)  (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p. */
//#define GET_SIZE(p)   (GET(p) & ~(DSIZE - 1))
#define GET_SIZE(p)   (GET(p))
#define GET_ALLOC(p)  (GET(p) & 0x1)

/* Given block ptr ptr, compute address of its header and footer. */
#define HDRP(ptr)  ((char *)(ptr) - WSIZE)
#define FTRP(ptr)  ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - WSIZE)  //time 2 because header and footer

/* Given block ptr ptr, compute address of next and previous blocks. */
#define NEXT_BLKP(ptr)  ((char *)(ptr) + GET_SIZE(((char *)(ptr) - WSIZE)))
#define PREV_BLKP(ptr) ((void *)(ptr) - GET_SIZE((void *)(ptr) - WSIZE))


// Global variables 
static char *head_of_mem; // head of the memory block
static char *head_of_freelist; // current head of the free list
static int fit_flag=0; // 0 for first_fit, 1 for next_fit and 2 for best_fit
static int freelist_size;

void* get_next_free(void *ptr);
static void coalesce(void);

void* get_next_free(void *ptr)
{
	void *new_ptr = '\0';
    printf("Pointer address %p\n", ptr);
    printf("Next address %p\n", (void *)GET(FTRP(ptr)));
    if(GET(FTRP(ptr)) != 1) 
    {
        new_ptr = (void *)GET(FTRP(ptr));
        return (new_ptr); // if out of memory or no fitting size found return null
    }
    else 
        return NULL;
	
}

static void* first_fit(size_t adj_size)
{
	void *ptr;
    printf("address of head of free list inside ff 1: %p\n", (head_of_freelist));
	for (ptr = head_of_freelist; GET_SIZE(HDRP(ptr)) != 0; ptr = get_next_free(ptr)) 
    {
		if (adj_size <= (GET_SIZE(HDRP(ptr))-2*WSIZE)) 
        {
            printf("size of block found: %ld\n", GET_SIZE(HDRP(ptr)));
            printf("size needed: %ld\n", adj_size);	
			return (ptr);
        }
	}
    return (NULL); // if out of memory or no fitting size found return null
}


static void* next_fit(size_t adj_size, void* start_ptr)
{
	void *ptr;
    printf("address of head of free list inside ff 1: %p\n", (head_of_freelist));
	for (ptr = start_ptr; GET_ALLOC(HDRP(ptr)) == 0; ptr = get_next_free(ptr)) 
    {
		if (adj_size <= GET_SIZE(HDRP(ptr)))
            printf("size at head of free list inside ff 2: %ld\n", GET_SIZE(HDRP(head_of_freelist)));	
			return (ptr);
	}
	return (NULL); // if out of memory or no fitting size found return null
}


static void* best_fit(size_t adj_size)
{
	void *ptr;
    void *best_ptr='\0';
    printf("address of head of free list inside ff 1: %p\n", (head_of_freelist));
	for (ptr = head_of_freelist; GET_ALLOC(HDRP(ptr)) == 0; ptr = get_next_free(ptr)) 
    {
		if (adj_size <= GET_SIZE(HDRP(ptr))) {
            printf("size at head of free list inside ff 2: %ld\n", GET_SIZE(HDRP(head_of_freelist)));
            if (best_ptr != NULL)
                best_ptr = ptr;
            if(GET_SIZE(HDRP(best_ptr)) > GET_SIZE(HDRP(ptr)))
                best_ptr = ptr;
        }
        		//return (ptr);
	}
    return (best_ptr); // if out of memory or no fitting size found return null
}



void remove_from_freelist(void* ptr, size_t orig_size)
{
    void *tmp = NEXT_BLKP(ptr);
    printf("this is the address of what I want to remove from the freelist %p\n", ptr);
    printf("this is the address of next element in the list %p\n", tmp);
    printf("2size at head of free list: %ld\n", GET_SIZE(HDRP(head_of_freelist)));
    head_of_freelist = tmp + WSIZE;
    PUT(HDRP(head_of_freelist), orig_size - GET_SIZE(HDRP(ptr)) - WSIZE);
    freelist_size = GET_SIZE(HDRP(head_of_freelist));
    printf("3size at head of free list: %ld\n", GET_SIZE(HDRP(head_of_freelist)));
    printf("4size at head of free list: %d\n", freelist_size);
    // if(PREV_BLKP(ptr) == NULL)
    // {
    //     head_of_freelist = NEXT_BLKP(ptr);
    // }
    // else{
    //     tmp = PREV_BLKP(ptr);
    //     //NEXT_BLKP(tmp) = NEXT_BLKP(ptr); // need to find a way to assign this
    // }
    // tmp = NEXT_BLKP(ptr);
    // //PREV_BLKP(tmp) = PREV_BLKP(ptr); // need to find a way to assign this
}


void myinit(int allocAlg)
{
    // I hardcoded the padding at the end of memory

    printf("myinit\n");
    head_of_mem = malloc(1024*1024); // head of the free list
    printf("address of head of mem1: %p\n", (head_of_mem));   
    PUT(head_of_mem, 0); // alignment   
    // printf("address of head of mem: %ld\n", GET(head_of_mem));                   
	PUT(head_of_mem + (0 * WSIZE), PACK(WSIZE, 1)); /* Prologue header */ 
	PUT(head_of_mem + (1 * WSIZE), PACK(WSIZE, 1)); /* Prologue footer */ 
	PUT(head_of_mem + (2 * WSIZE), PACK(1024*1024-32, 0));     /* Epilogue header */
    PUT(head_of_mem + (1024*1024-WSIZE), PACK(0, 1));
    head_of_freelist = head_of_mem + (3 * WSIZE);
	//head_of_mem += (2*WSIZE); // memory point er points to the beginning of the payload
    printf("address of head of mem2: %p\n", (head_of_mem));
    printf("address of head of free list: %p\n", (head_of_freelist));
    printf("alloc bit at head of free list:%ld\n", GET_ALLOC(HDRP(head_of_freelist)));
    printf("size at head of free list: %ld\n", GET_SIZE(HDRP(head_of_freelist)));
    freelist_size =GET_SIZE(HDRP(head_of_freelist));
    fit_flag = allocAlg;  
}

void mycleanup(void)
{
    printf("mycleanup\n");
    free(head_of_mem);
}

void* mymalloc(size_t size)
{
    printf("mymalloc\n");
    size_t adj_size = 0;
    void *ptr;

    //Adjust the size to be in standard size blocks
    if(size==0)
    {
        printf("zero size returned\n");
        return NULL;
    }
    if(size < WSIZE)
    {
        printf("hello1\n");
        adj_size = WSIZE;
        printf("adjusted size1: %ld\n", adj_size);
    }
    else if(size % (WSIZE) != 0)
    {
        adj_size = WSIZE * ((size + WSIZE + (WSIZE - 1)) / WSIZE);
        printf("adjusted size:2 %ld\n", adj_size);
    }
    else
    {
        adj_size = size;
        printf("adjusted size3: %ld\n", adj_size);
    }

    //Search for available space that will fit the adj_size in our mem block 
    if(fit_flag==0 && (ptr = first_fit(adj_size)) != NULL)
    {   
        printf("fit found\n");
        printf("\nget size of ptr block %ld and adj_size %ld\n\n", GET_SIZE(HDRP(ptr)), adj_size);
        printf("\nget pack size: %ld\n\n", PACK(adj_size, 1));
        
        if ((GET_SIZE(HDRP(ptr)) - (adj_size))>= (2 * WSIZE)) 
        { 
            size_t orig_size = 0;
            orig_size = GET_SIZE(HDRP(ptr));  //get the free list size before updating size
            PUT(HDRP(ptr), adj_size);
            PUT(FTRP(ptr), 1);
            printf("\n\nget size of ptr block %ld and adj_size %ld\n\n", GET_SIZE(HDRP(ptr)), adj_size);
            remove_from_freelist(ptr, orig_size);						
        } 
        // else 
        // {
        //     PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 1));
        //     PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 1));
        //     remove_from_freelist(ptr, 0);						
        // }

    return ptr;

    }
    printf("%ld",adj_size);
    return ptr;

}

void myfree(void *ptr)
{

    if(ptr==NULL)
    {
        return;
    }
	//PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
	//PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
    //Adding the space back 
    freelist_size = GET_SIZE(HDRP(head_of_freelist)) + GET_SIZE(HDRP(ptr));
    //PUT(HDRP(head_of_freelist), GET_SIZE(HDRP(head_of_freelist)) + GET_SIZE(HDRP(ptr)));

   //PUT(HDRP(ptr), -1*GET_SIZE(HDRP(ptr)));
    //FTRP(ptr) = (char *)HDRP(head_of_freelist);
    printf("size of ptr: %ld\n", GET_SIZE(HDRP(ptr)));
    printf("address of freed block: %p\n", ptr);
    printf("address of head of free list before free: %p\n", (head_of_freelist));
    PUT(FTRP(ptr), head_of_freelist);
    //memcpy(FTRP(ptr), (void *)(&head_of_freelist), 8);
    //tmp = FTRP(ptr);
    //memcpy(&tmp, head_of_freelist, 8);  //this is adding the free block to the front of freelist
    head_of_freelist = ptr;
    printf("address of head of free list AFTER free: %p\n", (head_of_freelist));
    printf("new explicit address of next element: %p\n", (void *)GET(FTRP(ptr)));

    coalesce();
    /*
    Need to put back into freelist
    */

	//coalesce(bp);
}


void* myrealloc(void *ptr, size_t size)
{	
	size_t curr_size;
    size_t realloced_size;
	void *realloced_ptr;

    if(size < 0)
    {
        perror("realloc failed for negative size\n");
    }
	
	if (size == 0) 
    {
		myfree(ptr);
		return NULL;
	}
	curr_size = GET_SIZE(HDRP(ptr));
	realloced_size = size + (2 * WSIZE); // adding 2*WSIZE extra for the header and footer

	
	
	if (realloced_size > curr_size)
    {
        if(GET_ALLOC(HDRP(NEXT_BLKP(ptr))) == 0 &&                              // checking if the next block is free, and current block size +
        curr_size + GET_SIZE(HDRP(NEXT_BLKP(ptr))) > realloced_size &&          // next block size is greater than requested size but no greater than
        realloced_size < curr_size + GET_SIZE(HDRP(NEXT_BLKP(ptr))) + 4*WSIZE)  // 4*WSIZE so we don't allocate a big chunk for a small request
		{
			remove_from_freelist(NEXT_BLKP(ptr), 0); // WILL 0 WORK????
			PUT(HDRP(ptr),PACK(curr_size + GET_SIZE(HDRP(NEXT_BLKP(ptr))),1));
			return ptr;
		}
		else
        {
			realloced_ptr = mymalloc(realloced_size); // if next block is not free or current size + next block size are not big enough
			if (realloced_ptr == NULL)
				return NULL;

            if ((GET_SIZE(HDRP(realloced_ptr)) - (realloced_size))>= 4*WSIZE) // I am not sure about this next block. I know I need to do something like this
            {                                                                 //  but copied this from mymalloc since I think we need similar type of functionality
                size_t orig_size = 0;
                orig_size = GET_SIZE(HDRP(realloced_ptr));  //get the free list size before updating size
                PUT(HDRP(ptr), PACK(realloced_size, 1)); // CHECK OUT FoR +1
                //PUT(HDRP(ptr), PACK(adj_size, 1)); // CHECK OUT FoR +1
                PUT(FTRP(ptr), 1);
                printf("\n\nget size of reallocated block %ld and reallocated_size %ld\n\n", GET_SIZE(HDRP(ptr)), realloced_size);
                remove_from_freelist(realloced_ptr, orig_size);						
                //ptr = NEXT_BLKP(ptr);
                //PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)) - adj_size, 0));   
                //PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)) - adj_size, 0));
                //coalesce(ptr);
            } 
            else 
            {
                PUT(HDRP(realloced_ptr), PACK(GET_SIZE(HDRP(realloced_ptr)), 1));
                PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(realloced_ptr)), 1));
                remove_from_freelist(realloced_ptr, 0);						
            }


			memcpy(realloced_ptr, ptr, curr_size);
			myfree(ptr);
			return realloced_ptr;
		}
    }
	else if(curr_size == realloced_size)
    {
        return ptr;
    }
    else
    {	
		if(curr_size - realloced_size < 4*WSIZE)
        {
			return ptr;
		}
        else
        {
            PUT(HDRP(ptr),PACK(realloced_size, 1));
            //PUT(FTRP(ptr),PACK(realloced_size, 1));
            PUT(HDRP(NEXT_BLKP(ptr)),PACK((curr_size - realloced_size), 0));
            myfree(NEXT_BLKP(ptr));
            PUT(FTRP(ptr), GET(head_of_freelist));  //this is adding the free block to the front of freelist
	        head_of_freelist = ptr;
            return ptr;
        }	
	}
}


static void coalesce(void)
{
	void *start = '\0';
    void *next = '\0';
    start = head_of_freelist;
    
    next = get_next_free(start);
    if(NEXT_BLKP(start) == next) 
    {
        PUT(HDRP(start), GET_SIZE(HDRP(start))+GET_SIZE(HDRP(next))+2*WSIZE);
    }

    while (get_next_free(start) != NULL)
    {
        start = get_next_free(start);
        next = head_of_freelist;
        while (get_next_free(next) != NULL)
        {
            next = get_next_free(next);
            if(NEXT_BLKP(start) == next) 
            {
                PUT(HDRP(start), GET_SIZE(HDRP(start))+GET_SIZE(HDRP(next))+2*WSIZE);
            }
        }      
    }

}



