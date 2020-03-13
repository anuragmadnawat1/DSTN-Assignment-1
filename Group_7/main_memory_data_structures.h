//frame of size 1KB
typedef struct frame
{
    char byte_array[1024];
}frame;

typedef struct frame_table_entry
{
    unsigned int valid:1;
    unsigned int pid;
    unsigned int page_number;

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
    frame_table_entry* frame_table;

    free_frame_list_dummy_head *ffl_dummy_head;
    free_frame *ffl_tail;

    used_frame_list_dummy_head *ufl_dummy_head;
    used_frame *recently_used_frame;

    frame *frame_array;
}main_memory;

typedef struct page_table_entry
{
    int page_base_address;
    unsigned int global_page:1;
    unsigned int modified:1;
    unsigned int referenced:1;
    unsigned int cache_disabled:1;
    unsigned int write:1;
    
}page_table_entry;