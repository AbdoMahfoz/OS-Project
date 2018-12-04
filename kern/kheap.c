#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>
#define heap_frames 40959
#define heap_page_tables 39
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

int heap_count[heap_frames];
uint32 phys_addr_table[heap_frames];
char* starting_address = KERNEL_HEAP_START;

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//NOTE: Allocation is based on NEXT FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	int page_count = (size / PAGE_SIZE) + (size % PAGE_SIZE != 0);
	int counter = 0;
	if(starting_address > (char*)KERNEL_HEAP_MAX)
	{
		starting_address = (char*)KERNEL_HEAP_START;
	}
	//cprintf("Requsting %d pages, %d bytes, starting addres = %x\n",page_count, size, starting_address);
	while(counter < heap_frames)//law malftsh el lafa kamla
	{
		int page_number;
		bool is_empty = 1;
		//cprintf("Ahead of starting address %d free pages\n", (((char*)KERNEL_HEAP_MAX - starting_address + 1) / PAGE_SIZE));
		if((((char*)KERNEL_HEAP_MAX - starting_address + 1) / PAGE_SIZE) < page_count)
		{
			//cprintf("->Circling back to start\n");
			counter += (((char*)KERNEL_HEAP_MAX  - starting_address + 1) / PAGE_SIZE) + 1;
			starting_address = (char*)KERNEL_HEAP_START;
			page_number = 0;
		}
		else
		{
			page_number = (starting_address - (char*)KERNEL_HEAP_START) / PAGE_SIZE;
			//cprintf("->Starting at page_number = %d\n", page_number);
		}
		//cprintf("->moving starting address to some free space\n");
		while(counter < heap_frames)
		{
			if(starting_address > (char*)KERNEL_HEAP_MAX)
			{
				starting_address = (char*)KERNEL_HEAP_START;
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
		if(counter >= heap_frames)
		{
			break;
		}
		//cprintf("->counter = %d, moved to %x\n",counter, starting_address);
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
			for(int i= 0;i<page_count;i++)
			{
				struct Frame_Info* x;
				allocate_frame(&x);
				phys_addr_table[page_number] = to_physical_address(x);
				map_frame(ptr_page_directory,x,starting_address,PERM_WRITEABLE);
				starting_address+=PAGE_SIZE;
				counter++;
				page_number++;
			}
		}
		if(is_empty)
		{
			//cprintf("Done: %x\n", evaluated_address);
			return (void*)evaluated_address;
		}
	}
	//cprintf("Fail, starting address = %x\n", starting_address);
	return NULL;
	//TODO: [PROJECT 2018 - BONUS1] Implement the BEST FIT strategy for Kernel allocation
	// Beside the NEXT FIT
	// use "isKHeapPlacementStrategyBESTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
	char* address = (char*)virtual_address;
	int page_number = (address - (char*)KERNEL_HEAP_START) / PAGE_SIZE;
	int temp = page_number;
	//cprintf("releasing %d pages from %x\n", heap_count[page_number], address);
	for(int i=0;i<heap_count[page_number];i++)
	{
		phys_addr_table[temp++] = 0;
		unmap_frame(ptr_page_directory,address);
		address +=PAGE_SIZE;
	}
	heap_count[page_number]=0;

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	for(int i = 0; i < heap_frames; i++)
	{
		if(phys_addr_table[i] == ROUNDDOWN(physical_address, PAGE_SIZE))
		{
			return KERNEL_HEAP_START + (i*PAGE_SIZE) + (physical_address & ((1 << 12) - 1));
		}
	}
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer

	return 0;

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	return phys_addr_table[(virtual_address - KERNEL_HEAP_START) / PAGE_SIZE] + (virtual_address & ((1 << 12) - 1));
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().
void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2018 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code


	panic("krealloc() is not implemented yet...!!");
	return NULL;

}
