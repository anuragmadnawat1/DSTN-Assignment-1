#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include "functions.h"

/*
PreConditions
Inputs:{pointer to main memory object, frame number which we need to add to free frame list}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}

Purpose of the Function: Add the frame number to the tail of the linked list of free frames and increment number of free frames

PostConditions
Updated free frame list with new entry
*/
void add_free_frame(main_memory* main_memory_object, int frame_number)
{

    main_memory_object->frame_table[frame_number].pid=-1;
    main_memory_object->frame_table[frame_number].page_number=-1;

    free_frame *new_frame = (free_frame *)malloc(sizeof(free_frame));
    new_frame->frame_number=frame_number;
    new_frame->next=NULL;
    main_memory_object->ffl_dummy_head->number_free_frames++;

    if(main_memory_object->ffl_dummy_head->next==NULL)
    {
        main_memory_object->ffl_dummy_head->next=new_frame;
        main_memory_object->ffl_tail=new_frame;
        return;
    }

    main_memory_object->ffl_tail->next=new_frame;
    main_memory_object->ffl_tail=new_frame;
}

/*
PreConditions
Inputs:{pointer to main memory object}


Purpose of the Function: Remove a free frame entry from the head of the free frame list and return the frame number. Decrements the number of free frames.

PostConditions
Updated free frame list upon removing entry
Return Value: 
frame number of the removed frame if the free frame list was not empty. {0<=frame number<number of frames in main memory}
-1, if free frame list was empty
*/
int remove_free_frame(main_memory* main_memory_object)
{

    if( main_memory_object->ffl_dummy_head->next==NULL)
    {
        return -1; //no free frame to return;
    }

    int frame_number =  main_memory_object->ffl_dummy_head->next->frame_number;
    
    free_frame * temp =  main_memory_object->ffl_dummy_head->next;
    main_memory_object->ffl_dummy_head->next= main_memory_object->ffl_dummy_head->next->next;
    free(temp);
    main_memory_object->ffl_dummy_head->number_free_frames--;

    return frame_number;
}

/*
PreConditions
Inputs:{pointer to main memory object, frame number which we need to add to the used frame list}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}

Purpose of the Function: Add the {frame number, reference bit} to linked list of used frames(fifo queue) and increment number of used frames. Make the currently added frame as the recently used frame

PostConditions
Updated used frame list with new entry
*/
void add_used_frame(main_memory* main_memory_object, int frame_number)
{
    
    used_frame *new_frame = (used_frame *)malloc(sizeof(free_frame));
    new_frame->frame_number=frame_number;
    new_frame->reference_bit=0;
    new_frame->next=NULL;
    main_memory_object->ufl_dummy_head->number_used_frames++;
    //head->number_used_frames++;

    if(main_memory_object->ufl_dummy_head->next==NULL)
    {
        main_memory_object->ufl_dummy_head->next=new_frame;
        new_frame->next=new_frame;

        main_memory_object->recently_used_frame=new_frame;
        
        main_memory_object->ufl_dummy_head->next=main_memory_object->recently_used_frame;
        return;
    }

    new_frame->next=main_memory_object->recently_used_frame->next;
    main_memory_object->recently_used_frame->next=new_frame;
    main_memory_object->recently_used_frame=new_frame;

    main_memory_object->ufl_dummy_head->next=main_memory_object->recently_used_frame;

}

/*
PreConditions
Inputs:{pointer to main memory object}

Purpose of the Function: The functions traverses the used frame list starting from the element after the recently used frame. This element is the frame which was used first (as it is a fifo circular queue). If the reference bit is set to 1, make it 0 and continue until you find an entry whose reference bit is 0. (giving second chance to a frame). This frame is removed from the list and returned. The recently used frame is updated appropriately

PostConditions
Updated used frame list upon removing entry
Return Value: 
frame number of the removed frame if the used frame list was not empty. {0<=frame number<number of frames in main memory}
-1, if the used frame list was empty
*/
int remove_used_frame(main_memory* main_memory_object)
{
    
    if(main_memory_object->ufl_dummy_head->next==NULL || main_memory_object->ufl_dummy_head->number_used_frames==0)
    {
        return -1;
    }

   //fprintf(stderr,"IN REMOVE USED FRAME\n");

    while(main_memory_object->recently_used_frame->next->reference_bit!=0)
    {
        main_memory_object->recently_used_frame->next->reference_bit=0;
        main_memory_object->recently_used_frame=main_memory_object->recently_used_frame->next;
    }

    int frame_number = main_memory_object->recently_used_frame->next->frame_number;


    used_frame *temp = main_memory_object->recently_used_frame->next;
    main_memory_object->recently_used_frame->next=main_memory_object->recently_used_frame->next->next;
    free(temp);

    main_memory_object->ufl_dummy_head->number_used_frames--;

    main_memory_object->ufl_dummy_head->next=main_memory_object->recently_used_frame;
    
    return frame_number;

}


/*
PreConditions
Inputs:{pointer to main memory object, frame number which we needs to be transfered from used frame list to free frame list}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}
frame number entry is present in used frame list

Purpose of the Function: Traverse the used frame list and remove the entry corresponding to frame_number. Also add this frame_number to the free frame list.

PostConditions
Updated used frame list and free frame list
*/
void transfer_to_free_frame_list(main_memory* main_memory_object, int frame_number)
{
    used_frame *walker = main_memory_object->ufl_dummy_head->next;
    used_frame *start = walker;

    while(walker!=NULL && walker->next != start)
    {
        if(walker->next->frame_number==frame_number)
        {
            used_frame* temp = walker->next;
            walker->next = walker->next->next;
            free(temp);
            main_memory_object->ufl_dummy_head->number_used_frames--;

            add_free_frame(main_memory_object,frame_number);
        }
        walker=walker->next;
    }

}


/*
PreConditions
Inputs:{pointer to main memory object, frame number whose reference bit must be set in the used frame list}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}
frame number entry is present in used frame list

Purpose of the Function: Traverse the used frame list and set the reference bit of that entry to 1.

PostConditions
Updated used frame list.
*/
void set_reference_bit(main_memory* main_memory_object,int frame_number)
{
    if(main_memory_object->ufl_dummy_head->next==NULL)
    {
        return;
    }

    used_frame* walker = main_memory_object->ufl_dummy_head->next;

    while(walker->frame_number!=frame_number)
    {
        walker=walker->next;
    }
    walker->reference_bit=1;
    return;
}


/*
PreConditions
Inputs:{pointer to the kernel object,pointer to main memory object}

Purpose of the Function: This function is called by a process to get a frame for itself when needed. The function first tries to get a frame from the free frame list(using get_free_frame method)(as placement is preferred over replacement). If we correctly received the frame from the free frame list, the function returns the frame number. If the free frame list is empty, the last used frame is extracted from the used frame list (using get_used_frame method). Upon receiving the used frame we must find out the pid of the proccess currently using the frame and the logical page of that process which was using it. Using this information we can invalidate the entry for this frame in the page table of that process (using invalidate_page_table_entry method)

PostConditions
Return Value: 
frame number of the removed frame. {0<=frame number<number of frames in main memory}
*/
int get_frame(kernel* kernel_object,main_memory *main_memory_object, int is_page_table, int brought_in_before)
{
    //add page fault times
    total_time_taken=total_time_taken+page_fault_overhead_time+restart_overhead_time;

    //we are getting frame for a page table
    if(is_page_table==1)
    {
        //as it is an in memory DS 
        if(brought_in_before==0)
        {
            total_time_taken=total_time_taken; //we initialize the page table, dont swap it
        }
        else if(brought_in_before==1)
        {
            total_time_taken=total_time_taken+swap_space_to_main_memory_transfer_time; //swap in the page
        }
    }
    else if(is_page_table==0)
    {
        //we get the page for the first time from the disk
        if(brought_in_before==0)
        {
            total_time_taken=total_time_taken + disk_to_main_memory_transfer_time;
        }
        //we get the page from the swap space
        else if(brought_in_before==1)
        {
            total_time_taken=total_time_taken+swap_space_to_main_memory_transfer_time; //swap in the page
        }
    }

    //prefer placement over replacement
    int frame_number = remove_free_frame(main_memory_object);

    if(frame_number!=-1)
    {
        add_used_frame(main_memory_object,frame_number);
        return frame_number;
    }

    fprintf(output_fd,"REMOVING USED FRAME\n");

    print_ufl(main_memory_object);
    frame_number = remove_used_frame(main_memory_object);
    print_ufl(main_memory_object);
    //need to update the page table of the process from which this was taken from and frame table of the OS

    int current_pid_of_frame = get_pid_of_frame(main_memory_object,frame_number);
    int current_logical_page_of_frame = get_page_number_of_frame(main_memory_object,frame_number);

    //invalidate the frame table entry for this frame
    main_memory_object->frame_table[frame_number].pid=-1;
    //main_memory_object->frame_table[frame_number].pointer_to_stored_page_table=NULL;

    //invalidate the page table entry for the pid and logical page we just got

    invalidate_page_table_entry(kernel_object,main_memory_object,current_pid_of_frame,current_logical_page_of_frame);

    //if the frame that we got was modified, it must be swapped out
    if(main_memory_object->frame_table[frame_number].modified==1)
    {
        total_time_taken=total_time_taken+main_memory_to_swap_space_transer_time;
        main_memory_object->frame_table[frame_number].modified=0;
    }
    
   
    //add to used frame list
    add_used_frame(main_memory_object,frame_number);
    
    return frame_number;
}


/*
PreConditions
Inputs: {frame number that this page table will occupy}

Purpose of the Function: This function intializes a page table

PostConditions
Output: {pointer to intialized page table}
*/
page_table* initialize_page_table(int frame_number_occupied)
{
    int i;
    page_table* page_table_object = (page_table*)malloc(sizeof(page_table));
    page_table_object->frame_occupied=frame_number_occupied;
    page_table_object->page_table_entries = (page_table_entry*)malloc(sizeof(page_table_entry)*256); //there are 2^8 entries in each page table

    for(i=0;i<256;i++)
    {
        //page_table_object->page_table_entries[i].cache_disabled=1;
        page_table_object->page_table_entries[i].frame_base_address=-1;
        page_table_object->page_table_entries[i].initialized_before=0;
        //page_table_object->page_table_entries[i].referenced=0;
        page_table_object->page_table_entries[i].modified=0;
        page_table_object->page_table_entries[i].valid=0;
        page_table_object->page_table_entries[i].pointer_to_page_table=NULL;
    }

    return page_table_object;

}


/*
PreConditions
Inputs: {pointer to main memory object, frame number}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}

Purpose of the Function: This function is used to just access the OS frame table using the frame number as its index and return the pid of the process using that frame

PostConditions
Output:{pid of process which has acquired that frame}
*/
int get_pid_of_frame(main_memory* main_memory_object,int frame_number)
{
    return main_memory_object->frame_table[frame_number].pid;
}



/*
PreConditions
Inputs: {pointer to main memory object, frame number}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}

Purpose of the Function: This function is used to just access the OS frame table using the frame number as its index and return the logical page of the process using that frame holds

PostConditions
Output:{logical page of the process using that frame holds}
if the frame is a page table, returns -1
*/
int get_page_number_of_frame(main_memory* main_memory_object,int frame_number)
{
    return main_memory_object->frame_table[frame_number].page_number;
}


/*
PreConditions
Inputs: {pointer to main memory object, frame number}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}

Purpose of the Function: This function is used to just access the OS frame table using the frame number as its index and return the pointer to the page table which the frame holds, if any.

PostConditions
Output:{If the frame is a page table, returns pointer to page table. else returns NULL}
*/
// page_table* get_page_table_pointer_of_frame(main_memory* main_memory_object, int frame_number)
// {
//     return main_memory_object->frame_table[frame_number].pointer_to_stored_page_table;
// }




/*
PreConditions
Inputs: {pointer to main memory object, frame number, pid, page_number, pointer to page table object}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}

Purpose of the Function: This function is used to set the frame table entry indexed by the frame number

PostConditions
Updated OS frame table
*/
void update_frame_table_entry(main_memory* main_memory_object,int frame_number,int pid,int page_number)
{
    main_memory_object->frame_table[frame_number].pid=pid;
    main_memory_object->frame_table[frame_number].page_number=page_number;
    //main_memory_object->frame_table[frame_number].pointer_to_stored_page_table=page_table_object;
}


/*
PreConditions
Inputs: {pointer to main memory object, pid, frame number}
0<=frame number<number of frames in main memory{32,768 for 32MB main memory and 1KB frame size}

Purpose of the Function: Checks if the pid stored in the frame table entry matches the pid in the argument of the function. If it matches, the frame is owned by that process

PostConditions
Return Value:
1 if frame is owned by the process whose pid is given in the function argument
-1, otherwise
*/
int check_frame_ownership(main_memory* main_memory_object,int pid,int frame_number)
{
    if(frame_number<0)
    {
        return -1;
    }
    if(main_memory_object->frame_table[frame_number].pid==pid)
    {
        return 1;
    }
    
    return -1;
}


/*
PreConditions
Inputs: {pointer to kernel object, pointer to main memory object, pid, logical address process is requesting for }
0<=logical address< 2^32

Purpose of the Function: Starts from the outermost page table stored in the pcb array entry of the process in the kernel object. Goes through 3 levels of paging. If any page table is missing in the page walk, the function gets a frame for that page table an initializes the page table in that frame. In the end the function gets a frame to store the logical page of the function and stores its enty in the innermost page table.

PostConditions
Return Value:
Frame number assigned to the logical page requested by process
*/
int page_table_walk(kernel* kernel_object, main_memory* main_memory_object, int pid, int logical_address)
{
    fprintf(output_fd,"Starting page walk of PID: %d | Request: %x | Request: %d\n",pid,logical_address,logical_address);
    fflush(output_fd);
    //32 bit Virtual Address split as:  6 | 8 | 8 | 10 . Hence 3 level paging is required

    //get the logical page number
    int logical_page_number = get_logical_page_number(logical_address);

    //get frame of outermost page table
    int outer_page_table_frame_number = kernel_object->pcb_array[pid].outer_page_base_address;

    //check if you own frame, otherwise load new frame for the outermost page table

    int own_outer_page_table = check_frame_ownership(main_memory_object,pid,outer_page_table_frame_number);

    fprintf(output_fd,"Outer Page Table Frame Number: %d | Own it: %d\n",outer_page_table_frame_number,own_outer_page_table);
    fflush(output_fd);

    if(own_outer_page_table==-1)
    {
        outer_page_table_frame_number = get_frame(kernel_object,main_memory_object,1,kernel_object->pcb_array[pid].outer_page_base_address_initialized_before);

        //make this frame the outermost page table
        
        if(kernel_object->pcb_array[pid].outer_page_base_address_initialized_before==0)
        {
            page_table *outermost_page_table = initialize_page_table(outer_page_table_frame_number);
            kernel_object->pcb_array[pid].outer_page_table=outermost_page_table;
            kernel_object->pcb_array[pid].outer_page_base_address_initialized_before=1;
        }
        

        //update the frame table for the frame we got
        update_frame_table_entry(main_memory_object,outer_page_table_frame_number,pid,logical_page_number); 
        
        //add it to the pcb of this process
        kernel_object->pcb_array[pid].outer_page_base_address=outer_page_table_frame_number;
        
    }

    own_outer_page_table = check_frame_ownership(main_memory_object,pid,outer_page_table_frame_number);
    fprintf(output_fd,"Outer Page Table Frame Number: %d | Own it: %d\n",outer_page_table_frame_number,own_outer_page_table);
    fflush(output_fd);

    set_reference_bit(main_memory_object,outer_page_table_frame_number);

    page_table* outer_page_table = kernel_object->pcb_array[pid].outer_page_table;

    //page_table* outer_page_table = get_page_table_pointer_of_frame(main_memory_object,outer_page_table_frame_number);

    int outer_page_table_offset = get_outer_page_table_offset(logical_address); //first 6 bits are the offset in the outermost page table

    fprintf(output_fd,"Outer Page Table Offset: %d\n",outer_page_table_offset);
    fflush(output_fd);

    //go to the middle level of paging
    
    int middle_page_table_frame_number = outer_page_table->page_table_entries[outer_page_table_offset].frame_base_address;

    

    int own_middle_page_table = check_frame_ownership(main_memory_object,pid,middle_page_table_frame_number);

    fprintf(output_fd,"Middle Page Table Frame Number: %d | Own it: %d\n",middle_page_table_frame_number, own_middle_page_table);
    fflush(output_fd);

    if(own_middle_page_table==-1 || outer_page_table->page_table_entries[outer_page_table_offset].valid==0)
    {
        middle_page_table_frame_number = get_frame(kernel_object,main_memory_object,1,outer_page_table->page_table_entries[outer_page_table_offset].initialized_before);

        //make this frame the middle page table
        if(outer_page_table->page_table_entries[outer_page_table_offset].initialized_before==0)
        {
            page_table* middle_page_table = initialize_page_table(middle_page_table_frame_number);
            outer_page_table->page_table_entries[outer_page_table_offset].pointer_to_page_table=middle_page_table;
            outer_page_table->page_table_entries[outer_page_table_offset].initialized_before=1;
        }
      

        //update frame table entry
        //update the frame table for the frame we got
        update_frame_table_entry(main_memory_object,middle_page_table_frame_number,pid,logical_page_number); 

        //link it from outermost page table
        outer_page_table->page_table_entries[outer_page_table_offset].frame_base_address=middle_page_table_frame_number;
        outer_page_table->page_table_entries[outer_page_table_offset].valid=1;
        outer_page_table->page_table_entries[outer_page_table_offset].modified=0;

    }

    own_middle_page_table = check_frame_ownership(main_memory_object,pid,middle_page_table_frame_number);

    fprintf(output_fd,"Middle Page Table Frame Number: %d | Own it: %d\n",middle_page_table_frame_number, own_middle_page_table);
    fflush(output_fd);

    set_reference_bit(main_memory_object,middle_page_table_frame_number);

    page_table* middle_page_table = outer_page_table->page_table_entries[outer_page_table_offset].pointer_to_page_table;
    //page_table* middle_page_table = get_page_table_pointer_of_frame(main_memory_object,middle_page_table_frame_number);

    int middle_page_table_offset = get_middle_page_table_offset(logical_address);

    fprintf(output_fd,"Middle Page Table Offset: %d\n",middle_page_table_offset);
    fflush(output_fd);

    //go to inner page table

    int inner_page_table_frame_number = middle_page_table->page_table_entries[middle_page_table_offset].frame_base_address;

    int own_inner_page_table = check_frame_ownership(main_memory_object,pid,inner_page_table_frame_number);

    fprintf(output_fd,"Inner Page Table Frame Number: %d | Own it: %d\n",inner_page_table_frame_number, own_inner_page_table);
    fflush(output_fd);

    if(own_inner_page_table==-1 || middle_page_table->page_table_entries[middle_page_table_offset].valid==0)
    {
        inner_page_table_frame_number=get_frame(kernel_object,main_memory_object,1,middle_page_table->page_table_entries[middle_page_table_offset].initialized_before);

        //make this frame the inner page table
        if(middle_page_table->page_table_entries[middle_page_table_offset].initialized_before==0)
        {   
            page_table* inner_page_table = initialize_page_table(inner_page_table_frame_number);
            middle_page_table->page_table_entries[middle_page_table_offset].pointer_to_page_table=inner_page_table;
            middle_page_table->page_table_entries[middle_page_table_offset].initialized_before=1;
        }
       

        //update frame table entry
        //update the frame table for the frame we got
        update_frame_table_entry(main_memory_object,inner_page_table_frame_number,pid,logical_page_number); 

        //link it from middle page table
        middle_page_table->page_table_entries[middle_page_table_offset].frame_base_address=inner_page_table_frame_number;
        middle_page_table->page_table_entries[middle_page_table_offset].valid=1;
        middle_page_table->page_table_entries[middle_page_table_offset].modified=0;
        
    }

    own_inner_page_table = check_frame_ownership(main_memory_object,pid,inner_page_table_frame_number);

    fprintf(output_fd,"Inner Page Table Frame Number: %d | Own it: %d\n",inner_page_table_frame_number, own_inner_page_table);
    fflush(output_fd);

    set_reference_bit(main_memory_object,inner_page_table_frame_number);

    page_table* inner_page_table = middle_page_table->page_table_entries[middle_page_table_offset].pointer_to_page_table;

    //page_table* inner_page_table = get_page_table_pointer_of_frame(main_memory_object,inner_page_table_frame_number);

    int inner_page_table_offset = get_inner_page_table_offset(logical_address);

    fprintf(output_fd,"Inner Page Table Offset: %d\n",inner_page_table_offset);
    fflush(output_fd);

    //get the frame we needed to store this logical address
    int needed_frame_number = inner_page_table->page_table_entries[inner_page_table_offset].frame_base_address;

    int own_needed_frame = check_frame_ownership(main_memory_object,pid,needed_frame_number);

  
   

    fprintf(output_fd,"Logical Page Number: %d\n",logical_page_number);
    fflush(output_fd);

    fprintf(output_fd,"Needed Frame Number: %d | Own it: %d\n",needed_frame_number, own_needed_frame);
    fflush(output_fd);
    
    if(own_needed_frame==-1 || inner_page_table->page_table_entries[inner_page_table_offset].valid==0)
    {
        //insert this entry into the frame table and the page table of this process
        needed_frame_number = get_frame(kernel_object,main_memory_object,0,inner_page_table->page_table_entries[inner_page_table_offset].initialized_before);
        

        //insert entry into page table
        inner_page_table->page_table_entries[inner_page_table_offset].frame_base_address=needed_frame_number;
        //inner_page_table->page_table_entries[inner_page_table_offset].referenced=1;

        //update frame table
        update_frame_table_entry(main_memory_object,needed_frame_number,pid,logical_page_number); 

        //link it from inner page table
        inner_page_table->page_table_entries[inner_page_table_offset].frame_base_address=needed_frame_number;
        inner_page_table->page_table_entries[inner_page_table_offset].pointer_to_page_table=NULL;
        inner_page_table->page_table_entries[inner_page_table_offset].valid=1;
        inner_page_table->page_table_entries[inner_page_table_offset].modified=0;
        inner_page_table->page_table_entries[inner_page_table_offset].initialized_before=1;
    }

    own_needed_frame = check_frame_ownership(main_memory_object,pid,needed_frame_number);
    fprintf(output_fd,"Needed Frame Number: %d | Own it: %d\n",needed_frame_number, own_needed_frame);
    fflush(output_fd);

    set_reference_bit(main_memory_object,needed_frame_number);

    fprintf(output_fd,"\n\n");
    // fprintf(output_fd,"Print Outer Page Table\n");
    // print_page_table(outer_page_table);

    // fprintf(output_fd,"Print Middle Page Table\n");
    // print_page_table(middle_page_table);

    // fprintf(output_fd,"Print Inner Page Table\n");
    // print_page_table(inner_page_table);

    //print_frame_table(main_memory_object);
    fflush(output_fd);

    return needed_frame_number;
}

/*
PreConditions
Inputs: {pointer to kernel object, pointer to main memory object, pid of processes whose page we need to invalidate , the logical page we need to invalidate }

Purpose of the Function: Invalidate the page table entry of the given process and logcial page number

PostConditions
In the page table of the process whose pid we got, we have invalidated the entry for the given logical page.
*/
void invalidate_page_table_entry(kernel* kernel_object, main_memory* main_memory_object, int pid, int logical_page)
{
    if(pid<0 || logical_page<0)
    {
        return;
    }
    int logical_address = logical_page<<10;
    //32 bit Virtual Address split as:  6 | 8 | 8 | 10 . Hence 3 level paging is required

    //get frame of outermost page table
    int outer_page_table_frame_number = kernel_object->pcb_array[pid].outer_page_base_address;

    //we dont own the outer page itself, hence return
    if (outer_page_table_frame_number==-1)
    {
        return;
    }
    

    //check if you own frame, otherwise load new frame for the outermost page table

    int own_outer_page_table = check_frame_ownership(main_memory_object,pid,outer_page_table_frame_number);

    if(own_outer_page_table==-1)
    {
        kernel_object->pcb_array[pid].outer_page_base_address=-1;
        return;
    }
    //set_reference_bit(main_memory_object,outer_page_table_frame_number);

    page_table* outer_page_table = kernel_object->pcb_array[pid].outer_page_table;
    //page_table* outer_page_table = get_page_table_pointer_of_frame(main_memory_object,outer_page_table_frame_number);

    int outer_page_table_offset = get_outer_page_table_offset(logical_address); //first 6 bits are the offset in the outermost page table

    //go to the middle level of paging
    
    int middle_page_table_frame_number = outer_page_table->page_table_entries[outer_page_table_offset].frame_base_address;

    int own_middle_page_table = check_frame_ownership(main_memory_object,pid,middle_page_table_frame_number);

    //we cant reach the next level of paging, hence return
    if(own_middle_page_table==-1 || outer_page_table->page_table_entries[outer_page_table_offset].valid==0)
    {
        //invalidate the entry in case we dont own the frame
        outer_page_table->page_table_entries[outer_page_table_offset].valid=0;
        return;

    }

    //set_reference_bit(main_memory_object,middle_page_table_frame_number);

    page_table* middle_page_table = outer_page_table->page_table_entries[outer_page_table_offset].pointer_to_page_table;
    //page_table* middle_page_table = get_page_table_pointer_of_frame(main_memory_object,middle_page_table_frame_number);

    int middle_page_table_offset = get_middle_page_table_offset(logical_address);

    //go to inner page table

    int inner_page_table_frame_number = middle_page_table->page_table_entries[middle_page_table_offset].frame_base_address;

    int own_inner_page_table = check_frame_ownership(main_memory_object,pid,inner_page_table_frame_number);

    //we cant reach the next level of paging , hence return
    if(own_inner_page_table==-1 || middle_page_table->page_table_entries[middle_page_table_offset].valid==0)
    {
        //invalidate the entry in case we dont own the frame
        middle_page_table->page_table_entries[middle_page_table_offset].valid=0;
        return;
    }

    //set_reference_bit(main_memory_object,inner_page_table_frame_number);

    page_table* inner_page_table = middle_page_table->page_table_entries[middle_page_table_offset].pointer_to_page_table;

    //page_table* inner_page_table = get_page_table_pointer_of_frame(main_memory_object,inner_page_table_frame_number);

    int inner_page_table_offset = get_inner_page_table_offset(logical_address);

    //get the frame we needed to store this logical address
    int needed_frame_number = inner_page_table->page_table_entries[inner_page_table_offset].frame_base_address;

    int own_needed_frame = check_frame_ownership(main_memory_object,pid,needed_frame_number);


    //anyway the entry that we wanted to invalidate, is invalid or the frame it points to is not ours
    if(own_needed_frame==-1 || inner_page_table->page_table_entries[inner_page_table_offset].valid==0)
    {
        //invalidate the entry in case we dont own the frame
        inner_page_table->page_table_entries[inner_page_table_offset].valid=0;
        return;
    }

    //invalidate the entry
    inner_page_table->page_table_entries[inner_page_table_offset].valid=0;

    //set_reference_bit(main_memory_object,needed_frame_number);

}


/*
PreConditions
Inputs: {main_memory_size in MB, frame_size in KB}

Purpose of the Function: Initialze Main Memory Data Structure and all the components in the Main Memory like the frame table, free frame list, used frame list (with second chance)

PostConditions
Output: {pointer to intialized main memory}
*/
main_memory* initialize_main_memory(float main_memory_size, float frame_size)
{
    //find number of frames
    
    int number_of_frames=(main_memory_size/frame_size)*1024;

    fprintf(output_fd,"Number of Frames: %d\n",number_of_frames);

    main_memory* main_memory_object = (main_memory *)malloc(sizeof(main_memory));
    main_memory_object->number_of_frames=number_of_frames;

    //initialize the frames
    //main_memory_object->frame_array=(frame *)malloc(number_of_frames*sizeof(frame));

    //initialize frame table
    main_memory_object->frame_table=(frame_table_entry *)malloc(number_of_frames*sizeof(frame_table_entry));

    //all pages are invalid upon initialization
    for(int frame_number=0;frame_number<number_of_frames;frame_number++)
    {
        main_memory_object->frame_table[frame_number].pid=-1;
        main_memory_object->frame_table[frame_number].page_number=-1;
        main_memory_object->frame_table[frame_number].modified=0;
        //main_memory_object->frame_table[frame_number].pointer_to_stored_page_table=NULL;
    }

    //initialize free frame list
    main_memory_object->ffl_dummy_head = (free_frame_list_dummy_head *)malloc(sizeof(free_frame_list_dummy_head));
    main_memory_object->ffl_dummy_head->next=NULL;
    main_memory_object->ffl_dummy_head->number_free_frames=0;
    main_memory_object->ffl_tail=NULL;

    //initialize used frame list
    main_memory_object->ufl_dummy_head=(used_frame_list_dummy_head *)malloc(sizeof(used_frame_list_dummy_head));
    main_memory_object->ufl_dummy_head->next=NULL;
    main_memory_object->ufl_dummy_head->number_used_frames=0;
    main_memory_object->recently_used_frame=NULL;
    

    main_memory_object->ffl_dummy_head->number_free_frames=number_of_frames; //all frames are initially free;

    //add all frames to the free frame list
    for(int frame_number=0;frame_number<number_of_frames;frame_number++)
    {
        if(main_memory_object->frame_table[frame_number].pid==-1)
        {
            
            add_free_frame(main_memory_object,frame_number);
            //fprintf(stderr,"Tail: %d\n",main_memory_object->ffl_tail->frame_number);
        }
        
    }
    return main_memory_object;
}


void mark_frame_modified(main_memory* main_memory_object, int frame_number)
{
    main_memory_object->frame_table[frame_number].modified=1;
}


void print_page_table(page_table* page_table_object)
{
    fprintf(output_fd,"Printing Page Table\n");

    int i;
    for(i=0;i<256;i++)
    {
        fprintf(output_fd,"Index: %d | Frame Number: %d | Valid: %d\n",i,page_table_object->page_table_entries[i].frame_base_address,page_table_object->page_table_entries[i].valid);
    }

    fprintf(output_fd,"\n\n");
}


/*auxillary function to print the frame table, useful for debugging*/
void print_frame_table(main_memory* main_memory_object)
{
    fprintf(output_fd,"Printing OS Frame Table\n");

    int number_of_frames = main_memory_object->number_of_frames;

    for(int i=0;i<number_of_frames;i++)
    {
        //fprintf(output_fd,"Frame Number: %d | Page Number: %d | PID: %d | Modified: %d | PageTable Pointer: %p\n",i,main_memory_object->frame_table[i].page_number,main_memory_object->frame_table[i].pid, main_memory_object->frame_table[i].modified, main_memory_object->frame_table[i].pointer_to_stored_page_table);
    }

    fprintf(output_fd,"\n\n");
}

/*auxillary function to print the free frame list, useful for debugging*/
void print_ffl(main_memory *main_memory_object)
{
    fprintf(output_fd,"Printing Free Frame List\nNumber of Free Frames are: %d\n",main_memory_object->ffl_dummy_head->number_free_frames);
    free_frame *walker;
    walker = main_memory_object->ffl_dummy_head->next;
    while(walker->next!=NULL)
    {
        fprintf(output_fd,"{Frame Number: %d}-->",walker->frame_number);
        walker=walker->next;
    }

    fprintf(output_fd,"\n");
}

/*auxillary function to print the used frame list, useful for debugging*/
void print_ufl(main_memory *main_memory_object)
{
    fprintf(output_fd,"Printing Used Frame List\nNumber of Used Frames are: %d\n",main_memory_object->ufl_dummy_head->number_used_frames);
    used_frame *walker;
    
    walker = main_memory_object->ufl_dummy_head->next;
    used_frame *temp=walker;
    if(walker!=NULL)
        fprintf(output_fd,"Recently Used Frame: {Frame Number: %d, Reference Bit: %d}\n",main_memory_object->recently_used_frame->frame_number,main_memory_object->recently_used_frame->reference_bit);
    while(walker!=NULL && walker->next!=temp)
    {
        fprintf(output_fd,"{Frame Number: %d, Reference Bit: %d}-->",walker->frame_number, walker->reference_bit);
        walker=walker->next;
    }
    if(walker!=NULL)
    {
        fprintf(output_fd,"{Frame Number: %d, Reference Bit: %d}-->",walker->frame_number, walker->reference_bit);
    }

    fprintf(output_fd,"\n");
}