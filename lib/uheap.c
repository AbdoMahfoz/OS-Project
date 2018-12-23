#include <inc/lib.h>

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
#define uheap_frames 131072
int heap_count[uheap_frames];
char* starting_address = USER_HEAP_START;
void* malloc(uint32 size)
{
	//TODO: [PROJECT 2018 - MS2 - [4] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	// Steps:
	//	1) Implement NEXT FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyNEXTFIT()
	//to check the current strategy
	int page_count = (size / PAGE_SIZE) + (size % PAGE_SIZE != 0);
	if(page_count > uheap_frames)
	{
		return 0;
	}
	int counter = 0;
	if(starting_address > (char*)USER_HEAP_MAX)
	{
		starting_address = (char*)USER_HEAP_START;
	}
	//cprintf("Requsting %d pages, %d bytes, starting addres = %x\n",page_count, size, starting_address);
	while(counter < uheap_frames)//law malftsh el lafa kamla
	{
		int page_number;
		bool is_empty = 1;
		page_number = (starting_address - (char*)USER_HEAP_START) / PAGE_SIZE;
		//cprintf("->Starting at page_number = %d\n", page_number);
		//cprintf("->moving starting address to some free space\n");
		while(counter < uheap_frames)
		{
			if(starting_address > (char*)USER_HEAP_MAX)
			{
				starting_address = (char*)USER_HEAP_START;
				page_number = 0;
			}
			if(heap_count[page_number])
			{
				//cprintf("we got %d pages resevered at %x, jumping to ", heap_count[page_number], starting_address);
				starting_address += heap_count[page_number] * PAGE_SIZE;
				counter += heap_count[page_number];
				page_number += heap_count[page_number];
				//cprintf("%x\n", starting_address);
			}
			else
			{
				break;
			}
		}
		//cprintf("Ahead of starting address %d free pages\n", (((char*)KERNEL_HEAP_MAX - starting_address + 1) / PAGE_SIZE));
		if((((char*)USER_HEAP_MAX - starting_address + 1) / PAGE_SIZE) < page_count)
		{
			//cprintf("->Circling back to start\n");
			counter += (((char*)USER_HEAP_MAX  - starting_address + 1) / PAGE_SIZE) + 1;
			starting_address = (char*)USER_HEAP_START;
			page_number = 0;
		}
		if(counter >= uheap_frames)
		{
			break;
		}
		for(int i=0;i<page_count;i++)
		{
			if(heap_count[page_number+i]!=0)
			{
			   //cprintf("->found reserved area at page %d\n", page_number+i);
			   counter += i;
			   page_number += i;
			   starting_address += i * PAGE_SIZE;
			   is_empty = 0;
			   break;
			}
		}
		char* evaluated_address = starting_address;
		if(is_empty)
		{
			heap_count[page_number] = page_count;
			/*
			for(int i= 0;i<page_count;i++)
			{
				/*
				phys_addr_table[page_number] = to_physical_address(x);
				struct Frame_Info* x;
				allocate_frame(&x);
				map_frame(ptr_page_directory,x,starting_address,PERM_WRITEABLE);
				starting_address+=PAGE_SIZE;
				page_number++;
			}
			*/
			sys_allocateMem(evaluated_address, page_count);
			starting_address += PAGE_SIZE * page_count;
			//cprintf("Done: %x\n", evaluated_address);
			return (void*)evaluated_address;
		}
	}
	//cprintf("Fail, starting address = %x\n", starting_address);
	return NULL;

	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2018 - MS2 - [4] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you shold get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details
	char* address = (char*)virtual_address;
	int page_number = (address - (char*)USER_HEAP_START) / PAGE_SIZE;
	//cprintf("releasing %d pages from %x\n", heap_count[page_number], address);
	/*
	int temp = page_number;
	for(int i=0;i<heap_count[page_number];i++)
	{
		phys_addr_table[temp++] = 0;
		unmap_frame(ptr_page_directory,address);
		address +=PAGE_SIZE;
	}
	*/
	sys_freeMem((uint32)address, heap_count[page_number]);
	heap_count[page_number]=0;

}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().;

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.


void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2018 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	//panic("realloc() is not implemented yet...!!");
	if(virtual_address == 0)
	{
		return malloc(new_size);
	}
	if(new_size == 0)
	{
		free(virtual_address);
		return 0;
	}
	uint32 addr = (uint32)virtual_address;
	addr = ROUNDDOWN(addr, PAGE_SIZE);
	int page_number = (addr - USER_HEAP_START)/PAGE_SIZE;
	int oldSize = heap_count[page_number];
	int newSize = (new_size / PAGE_SIZE) + (new_size % PAGE_SIZE != 0);
	if(oldSize >= newSize)
	{
		return virtual_address;
	}
	else
	{
		int fPageNumber = page_number + oldSize;
		int bPageNumber = page_number - 1;
		int requiredPageCount = newSize - oldSize;
		bool f1 = 1, f2 = 1;
		int fc = 0, bc = 0;
		for(int i = 0; i < requiredPageCount && (f1 || f2); i++)
		{
			if(f1)
			{
				if(heap_count[fPageNumber + i])
				{
					f1 = 0;
				}
				else
				{
					fc++;
				}
			}
			if(f2)
			{
				if(heap_count[bPageNumber - i])
				{
					f2 = 0;
				}
				else
				{
					bc++;
				}
			}
		}
		//Forward extension
		if(fc >= requiredPageCount)
		{
			//cprintf("\noldSize = %d, newSize = %d, freeFrames = %d\n", oldSize, newSize, sys_calculate_free_frames());
			heap_count[page_number] = newSize;
			addr += oldSize * PAGE_SIZE;
			/*
			for(int i = 0; i < newSize - oldSize; i++)
			{
				//cprintf("bew\n");
				struct Frame_Info* x;
				allocate_frame(&x);
				phys_addr_table[fPageNumber++] = to_physical_address(x);
				map_frame(ptr_page_directory,x,(void*)addr,PERM_WRITEABLE);
				addr += PAGE_SIZE;
			}
			*/
			sys_allocateMem((uint32)addr, newSize - oldSize);
			return virtual_address;
		}
		//Shift and extend
		else if(fc + bc >= requiredPageCount)
		{
			bPageNumber -= requiredPageCount - fc - 1;
			heap_count[page_number] = 0;
			heap_count[bPageNumber] = newSize;
			char* cAddr =(char*)(USER_HEAP_START + (bPageNumber * PAGE_SIZE));
			sys_allocateMem((uint32)cAddr, requiredPageCount - fc);
			for(int i = 0; i < newSize * PAGE_SIZE; i++)
			{
				/*
				if(phys_addr_table[bPageNumber] == 0)
				{
					struct Frame_Info* x;
					allocate_frame(&x);
					phys_addr_table[bPageNumber] = to_physical_address(x);
					map_frame(ptr_page_directory,x,(void*)(cAddr + (i * PAGE_SIZE)),PERM_WRITEABLE);
				}
				*/
				cAddr[i] = ((char*)addr)[i];
				/*
				if(i != 0 && i % PAGE_SIZE == 0)
				{
					bPageNumber++;
				}
				*/
			}
			return (void*)cAddr;
		}
		//just call malloc already!
		else
		{
			char* cAddr = (char*)malloc(new_size);
			for(int i = 0; i < oldSize * PAGE_SIZE; i++)
			{
				cAddr[i] = ((char*)addr)[i];
			}
			free((void*)addr);
			return (void*)cAddr;
		}
	}
	return 0;
}



//==================================================================================//
//============================= OTHER FUNCTIONS ====================================//
//==================================================================================//

void expand(uint32 newSize)
{
}

void shrink(uint32 newSize)
{
}

void freeHeap(void* virtual_address)
{
	return;
}


//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//[] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//[[6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FIRST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//[[6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement FIRST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return NULL;
}

