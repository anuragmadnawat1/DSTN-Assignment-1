#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include "functions.h"

// Random Function to that returns 0 or 1 with 
// equal probability 
int rand50() 
{ 
	// rand() function will generate odd or even 
	// number with equal probability. If rand() 
	// generates odd number, the function will 
	// return 1 else it will return 0. 
	return rand() & 1; 
} 

// Random Function to that returns 1 with 75% 
// probability and 0 with 25% probability using 
// Bitwise OR 
int rand75() 
{ 
	return rand50() | rand50(); 
} 


int main()
{
   output_fd = fopen(output_file,"w"); 

   //load all time values
   FILE* time_fd = fopen(time_parameter_file,"r");
   for(int j=0;j<28;j++)
   {
      fscanf(time_fd,"%ld",&L1_tlb_search_time);
      fscanf(time_fd,"%ld",&L2_tlb_search_time);
      fscanf(time_fd,"%ld",&L1_tlb_miss_OS_overhead);
      fscanf(time_fd,"%ld",&L2_tlb_miss_OS_overhead);
      fscanf(time_fd,"%ld",&processor_to_from_L1_tlb_transfer_time);
      fscanf(time_fd,"%ld",&processor_to_from_L2_tlb_transfer_time);
      fscanf(time_fd,"%ld",&L1_tlb_to_from_L2_tlb_transfer_time);
      fscanf(time_fd,"%ld",&L1_cache_indexing_time);
      fscanf(time_fd,"%ld",&L1_cache_tag_comparison_time);
      fscanf(time_fd,"%ld",&L2_cache_search_time);
      fscanf(time_fd,"%ld",&processor_to_from_L1_cache_transfer_time);
      fscanf(time_fd,"%ld",&processor_to_from_L2_cache_transfer_time);
      fscanf(time_fd,"%ld",&L1_cache_to_from_L2_cache_transfer_time);
      fscanf(time_fd,"%ld",&L1_cache_to_from_main_memory_transfer_time);
      fscanf(time_fd,"%ld",&L2_cache_to_from_main_memory_transfer_time);
      fscanf(time_fd,"%ld",&L2_cache_write_buffer_search_time);
      fscanf(time_fd,"%ld",&L2_cache_to_L2_cache_write_buffer_transfer_time);
      fscanf(time_fd,"%ld",&L2_cache_write_buffer_to_main_memory_transfer_time);
      fscanf(time_fd,"%ld",&update_bit_in_main_memory_time);
      fscanf(time_fd,"%ld",&pcb_data_processor_to_from_main_memory_transfer_time);
      fscanf(time_fd,"%ld",&page_table_entry_processor_to_from_main_memory_transfer_time);
      fscanf(time_fd,"%ld",&update_frame_table_entry_time);
      fscanf(time_fd,"%ld",&average_disk_to_from_swap_space_transfer_time_entire_process);
      fscanf(time_fd,"%ld",&main_memory_to_swap_space_transer_time);
      fscanf(time_fd,"%ld",&swap_space_to_main_memory_transfer_time);
      fscanf(time_fd,"%ld",&page_fault_overhead_time);
      fscanf(time_fd,"%ld",&restart_overhead_time);
      fscanf(time_fd,"%ld",&context_switch_time);

   }

   fclose(time_fd);


   FILE* input_fd = fopen(input_file,"r");
   int max_number_of_processes,j;
   fscanf(input_fd,"%d",&max_number_of_processes);

   //initialize total time taken as 0
   total_time_taken=0;

   number_of_tlb_hits=0;
   total_tlb_accesses=0;

   number_of_L1_cache_hits=0;
   total_L1_cache_accesses=0;

   number_of_L2_cache_hits=0;
   total_L2_cache_accesses=0;

   number_of_cache_misses=0;
   total_cache_accesses=0;

   number_of_page_misses=0;
   number_of_page_accesses=0;

   number_of_L1_tlb_searches=0;
   number_of_L2_tlb_searches=0;
   
   
   kernel* kernel_object = initialize_kernel(max_number_of_processes);
   main_memory* main_memory_32MB = initialize_main_memory(); 
   
   tlb* L1_tlb = initialize_tlb(12);
   tlb *L2_tlb = initialize_tlb(24);
   L1_cache* L1_instruction_cache_4KB = initialize_L1_cache();
   L1_cache* L1_data_cache_4KB = initialize_L1_cache();
   L2_cache* L2_cache_32KB = initialize_L2_cache();
   L2_cache_write_buffer* L2_cache_write_buffer_8 = initialize_L2_cache_write_buffer(8);

   //each process should ideally have these many frames
   number_of_frames_per_process_average = total_number_of_frames/max_number_of_processes;

   number_of_frames_per_process_lower_bound = number_of_frames_per_process_average/2;
   number_of_frames_per_process_upper_bound = number_of_frames_per_process_average+number_of_frames_per_process_lower_bound;

   long int number_of_requests_processed=0;
   unsigned int process_request;
   int executing_pid_index=0;
   int number_of_processes_ready=0;
   int pid_array[max_number_of_processes];


   int process_switch_instruction_count;
   fscanf(input_fd,"%d",&process_switch_instruction_count);
   
   //return 0;

   char filename[100]={0};
   char x;


   //load all the new processes
   for(j=0;j<max_number_of_processes;j++)
   {  
      fscanf(input_fd,"%c",&x);
      fscanf(input_fd,"%[^\n]",filename);
      fprintf(stderr,"Data from the file: %s\n", filename);

      load_new_process(kernel_object, main_memory_32MB,j,filename);

      fprintf(output_fd,"--------------------------------------------------------------------------------------\n");
      fflush(output_fd);

      fseek(kernel_object->pcb_array[j].fd,0,SEEK_SET);
      pid_array[j]=j;
      number_of_processes_ready++;

   }
   fclose(input_fd);
 

   int is_eof;
   int read_write;
   
   //int oldpid;
   int newpid;

   while(1)
   {
      if(number_of_requests_processed%process_switch_instruction_count==0)
      {
         //oldpid=pid_array[executing_pid_index];
         executing_pid_index=(executing_pid_index+1)%(number_of_processes_ready);
         newpid=pid_array[executing_pid_index];

         //context switch after every "process_switch_instruction_count" number of accesses
         context_switch(kernel_object,L1_tlb,L2_tlb,newpid);
      }

      is_eof=fscanf(kernel_object->pcb_array[pid_array[executing_pid_index]].fd,"%x",&process_request);

      //if we reach eof then process must be terminated
      if(is_eof==EOF)
      {
         //oldpid=pid_array[executing_pid_index];
         //terminate the process who has no more requests to process
         terminate_process(kernel_object,main_memory_32MB,pid_array[executing_pid_index]);

         //remove element of array pointed to by executing_pid_index
         fprintf(stderr,"Process %d Terminated\n",pid_array[executing_pid_index]);
         fprintf(output_fd,"Process %d Terminated\n",pid_array[executing_pid_index]);
         fflush(output_fd);
         fprintf(output_fd,"\n--------------------------------------------------------------------------------------\n");
         fflush(output_fd);
         
         
         for(j=executing_pid_index;j<number_of_processes_ready-1;j++)
         {
            pid_array[j]=pid_array[j+1];
         }
         number_of_processes_ready--;
         executing_pid_index=0;
         
         
         if(number_of_processes_ready==0)
         {
            fprintf(stderr,"Simulation Over\n");
            fprintf(output_fd,"Simulation Over\n");
            fflush(output_fd);
            
            break;
         }

         newpid=pid_array[executing_pid_index];
         context_switch(kernel_object,L1_tlb,L2_tlb,newpid);
         continue;
        
      }

      //if the process is requesting for an instruction, it can be Read Only
      if(get_request_type(process_request)==INSTRUCTION_REQUEST)
      {
         read_write=READ_REQUEST;
      }
      else
      {
         read_write=abs(1-rand75()); //this will produce 75% data requests as read and 25% data requests as write
      }
      
      //fprintf(stderr,"Executing request of PID: %d\n",pid_array[executing_pid_index]);

      execute_process_request(kernel_object,L1_tlb,L2_tlb,L1_instruction_cache_4KB,L1_data_cache_4KB,L2_cache_32KB,L2_cache_write_buffer_8,main_memory_32MB,pid_array[executing_pid_index],process_request,read_write);

      fprintf(output_fd,"\n--------------------------------------------------------------------------------------\n");
      fflush(output_fd);

      number_of_requests_processed++;
   }
   
   //printing simulation results

   fprintf(output_fd,"\n\nTotal Time Taken: %Lf ns\nNumber of Requests Processed: %ld\nEMAT: %Lf ns\n",total_time_taken,number_of_requests_processed,total_time_taken/number_of_requests_processed);
   fflush(output_fd);

   fprintf(stderr,"\n\nTotal Time Taken: %Lf\nNumber of Requests Processed: %ld\nEMAT: %Lf ns\n",total_time_taken,number_of_requests_processed,total_time_taken/number_of_requests_processed);

   tlb_hit_rate=number_of_tlb_hits/total_tlb_accesses;
   fprintf(output_fd,"TLB Hit Rate: %Lf%%\n",tlb_hit_rate*100);
   fflush(output_fd);
   fprintf(stderr,"TLB Hit Rate: %Lf%%\n",tlb_hit_rate*100);

   L1_cache_hit_rate=number_of_L1_cache_hits/total_L1_cache_accesses;
   fprintf(output_fd,"L1 Cache Hit Rate: %Lf%%\n",L1_cache_hit_rate*100);
   fflush(output_fd);
   fprintf(stderr,"L1 Cache Hit Rate: %Lf%%\n",L1_cache_hit_rate*100);

   L2_cache_hit_rate=number_of_L2_cache_hits/total_L2_cache_accesses;
   fprintf(output_fd,"L2 Cache Hit Rate: %Lf%%\n",L2_cache_hit_rate*100);
   fflush(output_fd);
   fprintf(stderr,"L2 Cache Hit Rate: %Lf%%\n",L2_cache_hit_rate*100);

   cache_hit_rate = 1 - (number_of_cache_misses/total_cache_accesses);
   fprintf(output_fd,"Overall Cache Hit Rate: %Lf%%\n",cache_hit_rate*100);
   fflush(output_fd);
   fprintf(stderr,"Overall Cache Hit Rate: %Lf%%\n",cache_hit_rate*100);

   page_fault_rate=number_of_page_misses/number_of_page_accesses;
   fprintf(output_fd,"Page Fault Rate: %Lf%%\n",page_fault_rate*100);
   fflush(output_fd);
   fprintf(stderr,"Page Fault Rate: %Lf%%\n",page_fault_rate*100);
   
   fclose(output_fd);
   

}

