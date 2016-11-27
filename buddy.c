/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	int id;
	int order;
	//maybe
	/* TODO: DECLARE NECESSARY MEMBER VARIABLES */
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		g_pages[i].order=0;
		g_pages[i].id=i;
		INIT_LIST_HEAD(&g_pages[i].list);
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
	g_pages[0].order =20;
}

/*
Made to get the id of the block
and break it apart piece by piece

*/
int alloc_id(int needed_order)
{
	page_t*  left;
	int id,right;
	int test;

	for (int i = needed_order; i <= MAX_ORDER; i++)
	{
		if (!list_empty(&free_area[i]))
		 {
			/* code */
			if(i == needed_order)
			{
				//get the struct and Id number
				left =list_entry(&free_area[i], page_t, list);
				id=left->id;
				left->order=needed_order;
				//remove it from this list
        
				list_del(&g_pages[id].list);
			}
			else
			{
					// recievsily get left-side
				id=alloc_id(needed_order +1);
				// get right side's id   based on lefts id and needed_order
				test= (1<<(needed_order -MIN_ORDER));
				right =id+test;
				// add right side to the appropriate list
				list_add(&g_pages[right].list, &free_area[needed_order]);



			}
		return id;
		}
	}
	return -1;
}
/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	int needed_order =MIN_ORDER;

	while ( size > (1<<needed_order) )
	{
		needed_order ++;
	}
	if(needed_order > MAX_ORDER)
		return NULL;


	int id = alloc_id(needed_order);
	if(id == -1)
		return NULL;
// get addr from id
return PAGE_TO_ADDR(id);

}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	int page_id =ADDR_TO_PAGE(addr);
	int order =g_pages[page_id].order;
	g_pages[page_id].order =0;
	void *buddy_addr =BUDDY_ADDR(addr, order);
	int buddy_id =ADDR_TO_PAGE(buddy_addr);
	while (g_pages[buddy_id].order == 0 && order < MAX_ORDER)
	{
		list_del(&g_pages[buddy_id].list);
		order ++;
		addr =  buddy_id >  page_id? addr : buddy_addr;
		page_id =ADDR_TO_PAGE(addr);
		buddy_addr = BUDDY_ADDR(addr, order);
		buddy_id = ADDR_TO_PAGE(buddy_addr);

	}
	list_add(&g_pages[page_id].list, &free_area[order]);




	/* TODO: IMPLEMENT THIS FUNCTION */
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}
