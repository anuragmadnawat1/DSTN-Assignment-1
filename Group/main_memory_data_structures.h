#ifndef MAIN_MEMORY_DATA_STRUCTURES_H
#define MAIN_MEMORY_DATA_STRUCTURES_H

struct page_table_entry;
typedef struct page_table_entry page_table_entry;

struct page_table;
typedef struct page_table page_table;

struct page_table_entry
{
    //assume each page table entry is 4 bytes
    
    page_table* pointer_to_page_table;
    int frame_base_address:15; //frame which is occupied by the page table pointed to by this entry
    int initialized_before:1;
    unsigned int global_page:1;
    unsigned int modified:1;
    unsigned int valid:1;
};

struct page_table
{
    int frame_occupied:15;

    //each page table will have 2^8 page table entries
    page_table_entry* page_table_entries;
};

typedef struct frame_table_entry
{
    //unsigned int valid:1;
    int page_number:22;
    int pid;
    unsigned int modified:1;
    //page_table* pointer_to_stored_page_table; //if the frame is a page table, then this is the pointer to that page table

}frame_table_entry;


struct free_frame_list
{
	unsigned int frame_number:15;
	struct free_frame_list *next;
};

typedef struct free_frame_list free_frame;

typedef struct free_frame_list_dummy_head
{
	int number_free_frames;
	free_frame *next;
}free_frame_list_dummy_head;


struct used_frame_list
{
	unsigned int frame_number:15;
    unsigned int reference_bit:1;
    //unsigned int pid;
	struct used_frame_list *next;
};

typedef struct used_frame_list used_frame;

typedef struct used_frame_list_dummy_head
{
	int number_used_frames;
	used_frame *next;
}used_frame_list_dummy_head;


typedef struct main_memory
{
    unsigned int number_of_frames;
    frame_table_entry* frame_table;

    free_frame_list_dummy_head *ffl_dummy_head;
    free_frame *ffl_tail;

    used_frame_list_dummy_head *ufl_dummy_head;
    used_frame *recently_used_frame;
  
   //frame *frame_array;
}main_memory;


#endif /*MAIN_MEMORY_DATA_STRUCTURES_H*/