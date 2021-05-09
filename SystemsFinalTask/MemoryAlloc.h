#include<stdio.h>
#include<stdlib.h>

#define PAGE_SIZE 1024



struct FreeMem
{
	void *start;		//start address in memory
	int size;			//size of block
	struct FreeMem* next;	//next node pointer
};


struct MemBlock
{
	struct FreeMem* ptr;	//pointer to first FreeMem node
	void* mem;				//pointer to hold the PAGE_SIZE amount of data
	int freespace;			//amount of space free in this block
};


//the memory space available
struct MemBlock memory[12];
int currMemBlock = 0;


//this function returns the block of memory where a particular address is present
int findBlock(void* ptr)
{

	void* startAddress = ((int*)ptr - 1);
	int size = *((int*)startAddress);
	void* endAddress = ((char*)startAddress + size + sizeof(int));

	for (int i = 0; i <= currMemBlock; i++)
	{
		void* blockEnd = ((char*)memory[i].mem + PAGE_SIZE);

		if (startAddress >= memory[i].mem  &&  memory[i].mem <= endAddress)
			return i;
	}

	return -1;
}

//this function initializes an index in the memory array 
void init(int index)
{
	if (index < currMemBlock)
	{
		return;
	}
	else
	{
		memory[index].mem = malloc(sizeof(PAGE_SIZE));
		memory[index].freespace = PAGE_SIZE;

		struct FreeMem* p = (struct FreeMem*)malloc(sizeof(struct FreeMem));
		p->next = NULL;
		p->size = memory[index].freespace;
		p->start = memory[index].mem;

		memory[index].ptr = p;
	}
}


//malloc function
void* AB_malloc(int size)
{
	int i;

	if (size > PAGE_SIZE - 4)
	{
		printf("Cannot Allocate\n");
		return NULL;
	}
	for (i = 0; i <= currMemBlock; i++)
	{
		//memory[i].freespace -= size;
		//memory[i].ptr;

		struct FreeMem* ptr = memory[i].ptr;
		while (ptr)
		{
			if (size <= ptr->size - sizeof(int))
			{
				ptr->size -= (size + sizeof(int));
				*((int*)ptr->start) = size;

				void* allocation = (void*)((int*)ptr->start + 1);

				ptr->start = ((char*)ptr->start + 4 + size);

				return allocation;
			}

			ptr = ptr->next;

		}


	}
	if (i >= currMemBlock)
	{
		init(++currMemBlock);
		return AB_malloc(size);
	}


}


//free function
void AB_free(void* ptr)
{
	//finding the block where address is present
	int index = findBlock(ptr);

	//data parameters
	void* startAddress = ((int*)ptr - 1);
	int size = *((int*)startAddress);
	void* endAddress = ((char*)startAddress + size + sizeof(int));


	//if that particular address is valid
	if (index != -1)
	{
		struct FreeMem* startPtr = memory[index].ptr;
		struct FreeMem* follow = NULL;

		//inserting at the beginning
		if (startPtr !=NULL && endAddress == startPtr->start)
		{
			startPtr->start = ptr;//startAddress;
			startPtr->size += (size + sizeof(int));

			return;
		}

		startPtr = NULL; int temp = 0;

		while (startPtr || temp == 0)
		{
			if (temp == 0)
			{
				startPtr = memory[index].ptr;
				temp++;
			}
			else
				startPtr = startPtr->next;

			if (endAddress <= startPtr->start)
			{
				break;
			}


			follow = startPtr;
		}


		if (startPtr == memory[index].ptr)
		{
			struct FreeMem* s = (struct FreeMem*)malloc(sizeof(struct FreeMem));
			s->next = memory[index].ptr;
			s->size = size;// +sizeof(int);
			s->start = startAddress;


			memory[index].ptr = s;
		}
		//inserting at the end
		else if (startPtr == NULL)
		{
			struct FreeMem* s = (struct FreeMem*)malloc(sizeof(struct FreeMem) * 1);
			s->next = NULL;
			s->size = size;
			s->start = startAddress;

			follow->next = s;
		}
		//inserting in middle
		else
		{

			void* followEnd = (void*)((char*)follow->start + follow->size + sizeof(int));
			void* startPtrStart = (void*)((char*)startPtr->start);// +sizeof(int));

			//combining 3 nodes
			if (startAddress == followEnd  &&  endAddress == startPtrStart)
			{
				follow->size = follow->size + startPtr->size + size;// +sizeof(int) + sizeof(int);
				follow->next = startPtr->next;

				//free(startPtr);
			}

			//combining left node
			else if (startAddress == followEnd)
			{
				follow->size = follow->size + size;// +sizeof(int);
			}

			//combining right node
			else if (endAddress == startPtrStart)
			{
				startPtr->start = startAddress;
				startPtr->size = startPtr->size + size;// +sizeof(int);

			}

		}



	}

}

//this function prints the given 
void printFreeList()
{

	for (int i = 0; i <= currMemBlock; i++)
	{
		struct FreeMem* ptr = memory[i].ptr;

		while (ptr)
		{
			printf("address = %p  size = %d \n", ptr->start, ptr->size);

			ptr = ptr->next;
		}
	}
}

