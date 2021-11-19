//Nathan Sexton

#if !defined MMHEADER_H
#define MMHEADER_H

#include <sys/mman.h>
#include<stdio.h>
#include <stdlib.h>

//structs and variables
typedef struct __mmalloc_t
{
	int size;
	int magic;
	double padding;

} mmalloc_t;

typedef struct __mmfree_t
{
	int size;
	struct __mmfree_t* next;

} mmfree_t;

mmfree_t* head;

//function declarations
void* mem_manager_malloc(int);
void mem_manager_free(void*);
void traverse_free_list(); //void traverse_free_list(mmfree_t* head)
void init_mem(int);
void* locate_split(mmfree_t*, int);
void* find_sorted_location(mmfree_t*, mmfree_t*);

//functions
void* mem_manager_malloc(int size)
{
	//iSize is the size of the current head
    int iSize = head->size;

	//s(total size) is the 16byte header plus size that's passed in
	int s = sizeof(mmalloc_t) + size;
	//setting void pointer var to whatever locate split returns
	void* var = locate_split(head, s);

	//head now equals var plus the size passed in
	//head's size is the initial size minus the size passed, in plus the 16 byte header
    head = (void*) var + size;
    head->size = iSize - s;

	return var;
}

void mem_manager_free(void* ptr)
{
	mmfree_t *header = (void*)ptr;
	int s = header->size;
	mmfree_t* fsl = (mmfree_t*) find_sorted_location(head, header);
	void* p;

	//if what's being inserted(header) is what's returned
	//set head->next to the old head
	//set new head to find sorted location       
	void* temp = (void*) head;
	if(fsl == header)
	{
		head = fsl;
		head->next = temp;
		head->size = s;
	}
	else
	{
		mmfree_t* temp = head;

		//while temp(head) isn't null
		while(temp)
		{
			if(temp == fsl)
			{
				//pointer after header is pointer after free
				header->next = fsl->next;
				//pointer after free is now header
				fsl->next = header;
			}
			else
			{
				temp = temp->next;
			}
			
		}
	}
	//COALESCE
	//while head isn't null, loop
	while(head)
	{
		//if head is by itself, leave the function
		if(head->next == NULL)
		{
			break;
		}

		//representing the pointer after head
		mmfree_t* headNext = head->next;
		//representing head, plus its size, to see if it equals the next pointer
		p = (void*)head + head->size + sizeof(mmfree_t);

		//if the pointer is equal to the next pointer
		if(p == (void*)headNext)
		{
			//get a new "next" variable and set it equal to head->next->next
			//now the head->next, equals that new next
			//now the size of head is the size of head plus the size of the pointer next to it
			void* newNext = headNext->next;
			head->next = newNext;
			head->size = head->size + headNext->size + sizeof(mmfree_t);
		}
		//if p isn't equal to head->next's location, set head equal to head->next
		//and if head->next is null, loop will break
		else
		{
			head = head->next;
		}
	}
}

//traverse the free space list starting from head
void traverse_free_list()
{
	
	if(!head)
	{
		printf("Invalid\n");
		return;
	}

	//temp variable so head doesn't get changed
	mmfree_t* temp = head;

	//printing mmfree list location and size of pointers
	while(temp)
	{
		printf("Pointer location: ");
		printf("%p\n", (void*) temp);
		printf("Free list size: %d\n\n", temp->size);
		temp = temp->next;
	}
}

//called by malloc
//find a free space chunk large enough for the requested allocation
//obtain some memory from that chunk
void* locate_split(mmfree_t* curr, int total_requested_size)
{
	//loop while curr isn't null
	while(curr)
	{
		//if curr size is greater than the total requested size, make malloc pointer that equals size of curr + 16byte header
		//then size of that pointer equals total requested size, minus its 16byte header
		//then return malloc pointer (as void type)
		if (curr->size > total_requested_size)
		{
			mmalloc_t* ptr = (void*)curr + sizeof(mmfree_t);
			ptr->size = total_requested_size - sizeof(mmalloc_t);
			return (void*) ptr;
		}
		//if the size of curr is already the total requested size, just return null
		else if(curr->size == total_requested_size)
		{
			return curr;
		}
		//if curr is less than, set curr its next pointer
		else
		{
			curr = curr->next;
		}
	}
}


//called by free
//locate the freed memory insert position so free space nodes are sorted by address
void* find_sorted_location(mmfree_t* head, mmfree_t* to_insert)
{
	//if the pointer that's being inserted is less than the header, return the pointer of what's being inserted
    if(to_insert < head)
    {
        return (void*) to_insert;
    }

	//while head isn't null
	while(head)
	{
		//checking that to insert is less than the pointer after head
		//if it is, return head
		if(to_insert < head->next)
		{
			return (void*)head;
		}
		//of value after head is null, return head
		else if (head->next == NULL)
		{
			return (void*)head;
		}
		//if it doesnt meet those conditions, keep iterating to the next pointer
		else
		{
			head = head->next;
		}
	}
}

void init_mem(int free_space_size)
{
	head = mmap(NULL, free_space_size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	//making size of where head is pointing, to be size of what's passed - 16byte buffer
	head->size = free_space_size - sizeof(mmfree_t);
	//pointer after head is null
	head->next = NULL;
	//print for how much space passed in 
	printf("Initialized Memory Size: %d\r\n\n", head->size);
}

#endif