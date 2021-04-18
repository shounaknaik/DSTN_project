#include <stdio.h>
#include <stdlib.h>
#include "l2_cache.h"


l2_cache* initialize_L2_cache () {

    // Create an empty L2 Cache structure
    l2_cache *l2_cache_1;
    l2_cache_1 = (l2_cache *) malloc (sizeof (l2_cache)); //  l1_tlb = (L1_TLB *) malloc (sizeof (L1_TLB));
    
    // Initialize the L2 cache with all entries marked INVALID
    for (int i = 0; i < NUM_L2_CACHE_SETS; i++) {
        for (int j = 0; j < NUM_L2_CACHE_WAYS ; j++)
            l2_cache_1->set_array[i].set_entries[j].valid_bit = INVALID;
    }
    
    return l2_cache_1;
}


data_byte* search_L2_cache (l2_cache* l2_cache_1, unsigned int physical_address,data_byte* write_data, int access_type) {

    // should search in main memory simultaneosly. As this is look aside.
    //int data=search_main_memory();
    int i=0;
    data_byte data[32];
    int byte_offset=physical_address%6;
    int useful_data=physical_address>>6;//6 bits is byte offset
    int set_index = useful_data % NUM_L2_CACHE_SETS;
    int tag = useful_data>> NUM_L2_CACHE_SET_INDEX_BITS;    
    
    for (i = 0; i < NUM_L2_CACHE_WAYS; i++) {
    
        // If the entry is VALID and the tag value matches the page entry tag bits -- Found!
        if (l2_cache_1->set_array[set_index].set_entries[i].valid_bit == VALID && l2_cache_1->set_array[set_index].set_entries[i].tag == tag) {
        
            // Get corresponding 64B data
            //printf ("Entry found in L2 Cache");
            break;
        }        
    }

    if (i < NUM_L2_CACHE_WAYS) {
        
        if(access_type==READ_ACCESS)
        {

            if(byte_offset<32)
            {
                for(int j=0;j<32;j++)
                {
                    data[j]=l2_cache_1->set_array[set_index].set_entries[i].data[j];
                }
            }
            else 
            {
                for(int j=0;j<32;j++)
                {
                    data[j+32]=l2_cache_1->set_array[set_index].set_entries[i].data[j];
                }

            }
            return data;

        }
        else 
        {
            //write. Write Through.
            //call main memory function.
             for(int j=0;j<64;j++)
            {
                    l2_cache_1->set_array[set_index].set_entries[i].data[j]=write_data[j];
            }
            //write same to MM
            
        }
      
    }
    else 
    {
         //Miss . Wait for Main Memory search. 
        update_l2_cache(l2_cache_1,data,set_index,tag);//Should get 64 bytes from MM
        
        return -73;

    }
}

void update_l2_cache (l2_cache* l2_cache_1, data_byte* data,int set_index,int tag) {
   
    int i = 0; 
    

    // PLACEMENT: If there are INVALID entries in L2 Cache, PLACE this entry in the first INVALID entry's slot    
    // Look for INVALID entry corresponding to the given set index
    for (i = 0; i < NUM_L2_CACHE_WAYS; i++) {
        if (l2_cache_1->set_array[set_index].set_entries[i].valid_bit == INVALID) {


            l2_cache_1->set_array[set_index].set_entries[i].tag = tag; 
            for(int j=0;j<64;j++)
            {
                l2_cache_1->set_array[set_index].set_entries[i].data[j]=data[j];
            }
            l2_cache_1->set_array[set_index].set_entries[i].valid_bit = VALID;
            l2_cache_1->set_array[set_index].set_entries[i].fifo_bits = 16;//15+1
            update_fifo_l2_cache(l2_cache_1,set_index);

            //must send updated data to main memory because it is write through
            break;
        }        
    }
    
    // REPLACEMENT: if all entries of the given set index are VALID, REPLACE by FIFO.
    if (i == NUM_L2_CACHE_WAYS) {
        int FIFO_replacement = get_FIFO_replacement (l2_cache_1, set_index);
        l2_cache_1->set_array[set_index].set_entries[FIFO_replacement].tag = tag; 
        for(int j=0;j<64;j++)
        {
            l2_cache_1->set_array[set_index].set_entries[FIFO_replacement].data[j]=data[j];
        }
        l2_cache_1->set_array[set_index].set_entries[FIFO_replacement].valid_bit = VALID;
        l2_cache_1->set_array[set_index].set_entries[FIFO_replacement].fifo_bits = 16;//15+1
        update_fifo_l2_cache(l2_cache_1,set_index);
    }
}

void update_fifo_l2_cache(l2_cache* l2_cache_1, int set_index)
{
    int i;
    for (i = 0; i < NUM_L2_CACHE_WAYS; i++) {
        if (l2_cache_1->set_array[set_index].set_entries[i].valid_bit == VALID) {
            l2_cache_1->set_array[set_index].set_entries[i].fifo_bits--;
        }        
    }
}
int get_FIFO_replacement(l2_cache* l2_cache_1,int set_index)
{
    for (int i = 0; i < NUM_L2_CACHE_WAYS; i++) {
        if (l2_cache_1->set_array[set_index].set_entries[i].fifo_bits == 0) {
            return i;
        }        
    }

}





