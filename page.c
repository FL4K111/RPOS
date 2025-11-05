#include "os.h"

//因为内存分配的单位是page, 所以需要考虑对齐问题

/*
 * Following global vars are defined in mem.S
 */
extern ptr_t DATA_START;
extern ptr_t DATA_END;
extern ptr_t BSS_START;
extern ptr_t BSS_END;
extern ptr_t HEAP_START;
extern ptr_t HEAP_SIZE;

/*
 * _alloc_start points to the actual start address of heap pool
 * _alloc_end points to the actual end address of heap pool
 * _num_pages holds the actual max number of pages we can allocate.
 */
static ptr_t _alloc_start = 0;
static ptr_t _alloc_end = 0;
static uint32_t _num_pages = 0;

static ptr_t _balloc_start = 0;
static ptr_t _num_bytes = 0;
static ptr_t _bytepool_start = 0;
static ptr_t _bytepool_end = 0;



//为实现字节粒度的内存分配，考虑在HEAP的高位取几页来专门作为字节分配的
//这是一种静态分配的方式
//实际的使用也是这样的，OS只负责粗粒度（页）级别的内存管理，即只会分配和回收页;
//而更小粒度，如字节级别的内存管理，由进程中的某一部分实现（一般在库文件中实现）。该部分
//从OS处申请页来专门作为分配字节时使用的空间，并通过位图来管理该空间。


#define PAGE_SIZE 4096
#define PAGE_ORDER 12

#define BYTE_PAGE_SIZE 8
#define LENGTH_RAM (512*1024)



#define PAGE_TAKEN (uint8_t)(1 << 0)
#define PAGE_LAST  (uint8_t)(1 << 1)


/*
 * Page Descriptor 
 * flags:
 * - bit 0: flag if this page is taken(allocated)
 * - bit 1: flag if this page is the last page of the memory block allocated
 */
struct Page {
	uint8_t flags;				//8比特的一个感觉可能是为了方便编程，因为C语言中最小的数据类型占8bit
};



static inline void _clear(struct Page *page)
{
	page->flags = 0;
}

static inline int _is_free(struct Page *page)
{
	if (page->flags & PAGE_TAKEN) {
		return 0;
	} else {
		return 1;
	}
}

static inline void _set_flag(struct Page *page, uint8_t flags)
{
	page->flags |= flags; //这是一种类似嵌入式的变成规范
}

static inline int _is_last(struct Page *page)
{
	if (page->flags & PAGE_LAST) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * align the address to the border of page(4K)
 */
static inline ptr_t _align_page(ptr_t address) //ptr_t = unsigned int
{
	ptr_t order = (1 << PAGE_ORDER) - 1;//PAGE_ORDER = 12
	return (address + order) & (~order); //可以理解为二进制形式的向上对齐，如10对齐，则11需变为20
}

//这里的向上对齐很有可能与后面通过从heap_star得到_alloc_start有关，在那里只能通过向上对齐实现
/*
 *    ______________________________HEAP_SIZE_______________________________
 *   /   ___num_reserved_pages___   ______________num_pages______________   \
 *  /   /                        \ /                                     \   \
 *  |---|<--Page-->|<--Page-->|...|<--Page-->|<--Page-->|......|<--Page-->|---|
 *  A   A                         A                                       A   A
 *  |   |                         |                                       |   |
 *  |   |                         |                                       |   _memory_end
 *  |   |                         |                                       |
 *  |   _heap_start_aligned       _alloc_start                            _alloc_end
 *  HEAP_START(BSS_END)
 *	
 *  Note: _alloc_end may equal to _memory_end.
 */

 //虽然在这里显示的是在对齐之后在分配额外的页来存储页表，但是在init中初始化的起始位置实际实在HEAP_START,即页表的起始是在内存对齐前的地址
void page_init()
{
	ptr_t _heap_start_aligned = _align_page(HEAP_START);


	/* 
	 * We reserved some Pages to hold the Page structures.
	 * The number of reserved pages depends on the LENGTH_RAM.
	 * For simplicity, the space we reserve here is just an approximation,
	 * assuming that it can accommodate the maximum LENGTH_RAM.
	 * We assume LENGTH_RAM should not be too small, ideally no less
	 * than 16M (i.e. PAGE_SIZE * PAGE_SIZE).
	 */
	//因为在rp2350中采用的是XIP,所以它的内存大小只有512kB,这样页表实际上只占用不到一页的位置，这就要求分配的时候至少给页表分配一页，不然就会出错
	uint32_t num_reserved_pages = LENGTH_RAM / (PAGE_SIZE * PAGE_SIZE);//（LENGTH / PAGE_SIZE) / (PAGE_SIZE / page_struct_size)
	//why use LENGTH_RAM, instead of HEAP_SIZE
	if (!num_reserved_pages){
		num_reserved_pages = 1;
	}


	_num_pages = (HEAP_SIZE - (_heap_start_aligned - HEAP_START))/ PAGE_SIZE - num_reserved_pages;//内存对齐丢弃的是head的低地址部分
	printf("HEAP_START = %p(aligned to %p), HEAP_SIZE = 0x%lx,\n"
	       "num of reserved pages = %d, num of pages to be allocated for heap = %d\n",
	       HEAP_START, _heap_start_aligned, HEAP_SIZE,
	       num_reserved_pages, _num_pages);
	
	/*
	 * We use HEAP_START, not _heap_start_aligned as begin address for
	 * allocating struct Page, because we have no requirement of alignment
	 * for position of struct Page.
	 */
	/*
	 *这里不用管是从HEAP_START 开始还是aligned开始，因为这部分对用户不可见，且始终会丢弃一部分内存；
	 *只要保证最后在内存分配的时候是从_alloc_start开始就行
	 */
	struct Page *page = (struct Page *)HEAP_START; 
	for (int i = 0; i < _num_pages; i++) { 
		_clear(page);
		page++;	
	}

	_alloc_start = _heap_start_aligned + num_reserved_pages * PAGE_SIZE;
	_alloc_end = _alloc_start + (PAGE_SIZE * _num_pages);
	//这里考虑的还是不够，需要提前在高地址区域分配一些页出来做更细粒度的分配

	printf("DATA:   %p -> %p\n", DATA_START, DATA_END);
	printf("BSS:    %p -> %p\n", BSS_START, BSS_END);
	printf("HEAP:   %p -> %p\n", _alloc_start, _alloc_end);
}

/*
 * Allocate a memory block which is composed of contiguous physical pages
 * - npages: the number of PAGE_SIZE pages to allocate
 */
void *page_alloc(int npages)
{
	/* Note we are searching the page descriptor bitmaps. */
	int found = 0;
	struct Page *page_i = (struct Page *)HEAP_START;
	for (int i = 0; i <= (_num_pages - npages); i++) {
		if (_is_free(page_i)) {
			found = 1;
			/* 
			 * meet a free page, continue to check if following
			 * (npages - 1) pages are also unallocated.
			 */
			struct Page *page_j = page_i + 1;
			for (int j = i + 1; j < (i + npages); j++) {
				if (!_is_free(page_j)) {
					found = 0;
					break;
				}
				page_j++;
			}
			/*
			 * get a memory block which is good enough for us,
			 * take housekeeping, then return the actual start
			 * address of the first page of this memory block
			 */
			if (found) {
				struct Page *page_k = page_i;
				for (int k = i; k < (i + npages); k++) {
					_set_flag(page_k, PAGE_TAKEN);
					page_k++;
				}
				page_k--;
				_set_flag(page_k, PAGE_LAST);
				return (void *)(_alloc_start + i * PAGE_SIZE);
			}
		}
		page_i++;
	}
	return NULL;
}

/*
 * Free the memory block
 * - p: start address of the memory block
 */
 //RVOS现在是单任务操作系统，不用考虑多任务的内存保护
void page_free(void *p)
{
	/*
	 * Assert (TBD) if p is invalid
	 */
	if (!p || (ptr_t)p >= _alloc_end) {
		return;
	}
	/* get the first page descriptor of this memory block */
	struct Page *page = (struct Page *)HEAP_START;
	page += ((ptr_t)p - _alloc_start)/ PAGE_SIZE;
	/* loop and clear all the page descriptors of the memory block */
	while (!_is_free(page)) {
		if (_is_last(page)) {
			_clear(page);
			break;
		} else {
			_clear(page);
			page++;;
		}
	}
}

void page_test()
{
	void *p = page_alloc(2);
	printf("p = %p\n", p);
	//page_free(p);

	void *p2 = page_alloc(7);
	printf("p2 = %p\n", p2);
	page_free(p2);

	void *p3 = page_alloc(4);
	printf("p3 = %p\n", p3);
}

void _byte_init()					//为了方便管理字节分配必须以16字节为单位；
{
	_bytepool_start = (ptr_t)page_alloc(BYTE_PAGE_SIZE);
	_bytepool_end = _bytepool_start + BYTE_PAGE_SIZE * PAGE_SIZE;
	uint32_t _num_reserved_bytes = BYTE_PAGE_SIZE * PAGE_SIZE / 16;
	_balloc_start = _bytepool_start + _num_reserved_bytes;
	_num_bytes = _bytepool_end - _balloc_start;

	uint8_t *p = (uint8_t *)_bytepool_start;
	for(int i = 0; i < _num_bytes / 16; i++)
	{
		*p = 0;
		p++;
	}
	printf("_bytepool_start = %p\n_bytepool_end = %p\n_balloc_start = %p\n_num_bytes = %d\n", _bytepool_start, _bytepool_end, _balloc_start, _num_bytes);
}

void *malloc(uint32_t size)
{
	if(!size)
		return NULL;
	uint32_t aligned_size = 0;
	if(size % 8)
	{
		aligned_size = ((size / 16) + 1) * 16;
	}
	else{
		aligned_size = size;
	}
	size = aligned_size;
	uint8_t *p_i = (uint8_t *)_bytepool_start;
	uint8_t *p_j = NULL;
	uint8_t found = 0;
	for(int i = 0; i < (_num_bytes - size) / 16; i++)
	{
		if(*p_i == 0)
		{
			found = 1;
			p_j = p_i + 1;
			for(int j = i + 1; j < i + (size) / 16; j++)
			{
				if(*p_j != 0)
				{
					found = 0;
					break;
				}
				p_j++;
			}

			if(found)
			{
				p_j = p_i;
				for(int j = 1; j < size / 16; j ++)
				{
					* p_j = 1;
					p_j++;
				}
				*p_j = 2;
				return (void *)(_balloc_start + i * 16);
			}
		}
		p_i++;
	}
	return NULL;

}
void free(void *p)
{
	if(!p || (ptr_t)p > _bytepool_end)
		return;
	uint8_t *p_t = (uint8_t *)_bytepool_start;
	p_t += (((ptr_t)p - _balloc_start) / 16);
	while(*p_t != 0)
	{
		if(*p_t == 2)
		{
			*p_t = 0;
			break;
		}
		else{
			*p_t = 0;
			p_t++;
		}
	}
}

void byte_test()
{
	void *p1 = malloc(1);
	printf("p1 = %p\n", p1);

	void *p2 = malloc(1);
	printf("p2 = %p\n", p2);

	free(p1);
	free(p2);
}

