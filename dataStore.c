/**
 * @file   dataStore.c
 * @author Pooh Cook
 * @date   20 April 2018
 * @version 0.1
 * @brief   A circular buffer for ints
 *      buffer will store value in place and read back out in a circular fashion 
 *      if storage is exceeded, values at tail will be lost  
 */

// these utilities are intended for use in a Linux Kernel Module but are unit tested in a 
// user mode program. The TEST_COMPILE switch is used to determine which mode to compile into
#ifndef TEST_COMPILE

#include <linux/module.h> 
#include <linux/slab.h>

#else

#include <stdlib.h>
#include <stdbool.h>
#include "testStubs.h"

#endif

#include "dataStore.h"

// returns the end of store buffer pointer 
static int* buffer_end(struct dataStore* ds ){
    
    return ds->buffer + ds->buff_size -1;
}

//  advance the indicated pointer forward accounting for circular storage
static void advance_p(struct dataStore* ds, int** p){
    
    if( *p == buffer_end(ds) )  
        *p = ds->buffer;
    else
        (*p)++;
}

//  regress the indicated pointer backward accounting for circular storage
static void rollback_p(struct dataStore* ds, int** p){
    
    if( *p == ds->buffer)  
        *p = buffer_end(ds);
    else
        (*p)--;
}

//  tell if space is available
static bool is_space_avaliable ( struct dataStore* ds ){
    
   return( !((ds->buff_inp == buffer_end(ds)) && (ds->buff_outp == ds->buffer)) &&
        (ds->buff_inp + 1 != ds->buff_outp ) );
           
}

// remove data from the head  if room is required
static void assure_space_avaliable ( struct dataStore* ds ){
    
   if( !is_space_avaliable(ds) ) 
       advance_p(ds, &(ds->buff_outp));
           
}

/** @brief This function is used to create a new data store of the specified size
 *          CAUTION: This functions allocates memory so make sure that FreeDataStore()
 *          Gets called in concert  
 *  
 *  @param buffer_size  size of the storage buffer to allocate
 *  @return pointer to new data store
 */
struct dataStore* CreateDataStore(int buffer_size){
    
    struct dataStore* ds = kmalloc(sizeof(struct dataStore), GFP_KERNEL);
    
    // HACK, in order to accommodate simple pointer handling
    // there has to be one empty char in a full buffer...  Suspect this
    // can be relieved    
    ds->buffer = kmalloc(sizeof(int[buffer_size+1]), GFP_KERNEL);
    ds->buff_size = buffer_size+1;
    ds->buff_inp = ds->buff_outp = ds->buffer;
    
    return ds;    
}

/** @brief This function is used free an allocated dataStore created by CreateDataStore
 *  
 *  @param ds   pointer to dataStore structure
 */
void FreeDataStore(struct dataStore* ds){  
    
        kfree(ds->buffer);
        kfree(ds);
}
/** @brief This function stores a single integer into a dataStore 
 *          NOTE: if the data store is full, then data will be lost from the head 
 *          to accommodate storage of the new values
 *  
 *  @param  ds     pointer to dataStore structure
 *  @param  value  value to be stored
 *  @return  always 0 (success)
 */
int StoreNumber( struct dataStore* ds, int value ){    
    
    assure_space_avaliable(ds);
    
    *(ds->buff_inp) = value;
    advance_p(ds, &(ds->buff_inp));
    
    return 0;
    
}

/** @brief This function pushes a single integer into the tail of the  dataStore 
 *          NOTE: if the data store is full, then the value will not be pushed
 *  
 *  @param  ds     pointer to dataStore structure
 *  @param  value  value to be stored
 *  @return  0 = success
 *           -1 = buffer full (value not pushed)
 */
int PushNumber( struct dataStore* ds, int value ){    
    
    if(!is_space_avaliable(ds))return -1; // don't overwrite head
    
    rollback_p(ds, &(ds->buff_outp));
    *(ds->buff_outp) = value;
        
    return 0;
    
}

/** @brief This function retrieves a single int from the head of the  dataStore 
 *  
 *  @param  ds     pointer to dataStore structure
 *  @param  value  pointer to where value is to be stored
 *  @return  0 = success
 *           -1 = buffer empty (no value returned)
 */
int RetriveNumber( struct dataStore* ds, int* value ){
    
    if(IsStoreEmpty(ds)) return -1;
    
    *value = *(ds->buff_outp);
    advance_p(ds, &(ds->buff_outp));
    
    return 0;
    
}

/** @brief This function checks if the Store is Empty 
 *  
 *  @param  ds     pointer to dataStore structure
 *  @return  false = not empty
 *           true = empty
 */
bool IsStoreEmpty( struct dataStore* ds){
    return( ds->buff_inp == ds->buff_outp);
}

/** @brief This function stores a a series of integers into a dataStore 
 *          NOTE: if the data store is full, then data will be lost from the head 
 *          to accommodate storage of the new values
 *  
 *  @param  ds     pointer to dataStore structure
 *  @param  values pointer to int array of values to be stored
 *  @param  count  number of values to store
 *  @return  always 0 (success)
 */
int StoreNumbers( struct dataStore* ds, int* values, int count ){    
    // TODO: This could be optimized into one or two memcpy ops and pointer adjust
    int indx;
    for ( indx=0 ; indx < count ; indx++){
        StoreNumber(ds, values[indx]);
    }
       
    return 0;    
}

/** @brief This function retrieves a series of ints from the head of the  dataStore 
 *           reads up tot he size of the buffer provided or until data is exausted 
 *           from the dataStore
 *  
 *  @param  ds     pointer to dataStore structure
 *  @param  value  pointer to array where values are to be stored
 *  @param  size   size of values array
 *  @return  count of ints writtten to bufffer
 */
 int RetriveNumbers( struct dataStore* ds, int* values, int size ){
    // TODO: This could be optimized into one or two memcpy ops and pointer adjust
    int rdIndx;
    for(  rdIndx = 0; !IsStoreEmpty(ds) && rdIndx < size; rdIndx++ ){
     RetriveNumber(ds, values+rdIndx);  
    }
    
    return rdIndx;    
}

     
/** @brief This function retrieves a series of ints from the head of the  dataStore 
 *           without removing any data from the dataStore. It reads up to the size 
 *           of the buffer provided or until data is exausted from the dataStore
 *  
 *  @param  ds     pointer to dataStore structure
 *  @param  value  pointer to array where values are to be stored
 *  @param  size   size of values array
 *  @return  count of ints writtten to bufffer
 */
 int PeekBuffer (struct dataStore* ds, int* values, int size ){
    
    int* buff_outp;                     // copy output pointer
    int rdIndx;
        
    buff_outp = ds->buff_outp;
    
    for(  rdIndx = 0; ds->buff_inp != buff_outp && rdIndx < size; rdIndx++ ){
        values[rdIndx] = *buff_outp;
        advance_p(ds, &buff_outp);
    }
    
    return rdIndx;
}


/** @brief This function pushes a series of integers into the tail of a dataStore 
 *          NOTE: if the data store is full, then the values will not be pushed
 *  
 *  @param  ds     pointer to dataStore structure
 *  @param  values pointer to int array of values to be stored
 *  @param  count  number of values to store
 *  @return  always 0 (success)
 */
 int PushBackNumbers( struct dataStore* ds, int* values, int count ){
    // TODO: This could be optimized into one or two memcpy ops and pointer adjust
    int indx;
    for ( indx=count-1 ; indx >= 0 ; indx--){
        if( PushNumber(ds, values[indx]) != 0) break;
    }
       
    return 0;    
}
