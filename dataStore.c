/**
 * @file   dataStore.c
 * @author Pooh Cook
 * @date   20 April 2018
 * @version 0.1
 * @brief   A circular buffer for ints
 *      buffer will store value in place and read back out in a circular fashion 
 *      if storage is exceeded, values at tail will be lost  
 */

#ifndef TEST_COMPILE

#include <linux/module.h> 
#include <linux/slab.h>
#include "dataStore.h"

#else

#include <stdlib.h>
#include <stdbool.h>
#include "dataStore.h"

#define GFP_KERNEL 0
void *kmalloc(size_t size, int flags){
  return malloc(size);   
}

void kfree( void * blk){
    free(blk);
}

#endif



static int* buffer_end(struct dataStore* ds ){
    
    return ds->buffer + ds->buff_size -1;
}

static void advance_p(struct dataStore* ds, int** p){
    
    if( *p == buffer_end(ds) )  
        *p = ds->buffer;
    else
        (*p)++;
}

static void assure_space_avaliable ( struct dataStore* ds ){
    
   if( ((ds->buff_inp == buffer_end(ds)) && (ds->buff_outp == ds->buffer)) ||
        (ds->buff_inp + 1 == ds->buff_outp ) ) 
       advance_p(ds, &(ds->buff_outp));
           
}

struct dataStore* CreateDataStore(int bufferSize){
    
    struct dataStore* ds = kmalloc(sizeof(struct dataStore), GFP_KERNEL);
    
    // HACK, in order to accommodate simple pointer handling
    // there has to be one empty char in a full buffer...  Suspect this
    // can be relieved    
    ds->buffer = kmalloc(bufferSize+1, GFP_KERNEL);
    ds->buff_size = bufferSize+1;
    ds->buff_inp = ds->buff_outp = ds->buffer;
    
    return ds;    
}

void FreeDataStore(struct dataStore* ds){  
    
        kfree(ds->buffer);
        kfree(ds);
}

int StoreNumber( struct dataStore* ds, int value ){    
    
    assure_space_avaliable(ds);
    
    *(ds->buff_inp) = value;
    advance_p(ds, &(ds->buff_inp));
    
    return 0;
    
}

int RetriveNumber( struct dataStore* ds, int* outValue ){
    
    if(IsStoreEmpty(ds)) return -1;
    
    *outValue = *(ds->buff_outp);
    advance_p(ds, &(ds->buff_outp));
    
    return 0;
    
}

bool IsStoreEmpty( struct dataStore* ds){
    return( ds->buff_inp == ds->buff_outp);
}

int StoreNumbers( struct dataStore* ds, int* values, int count ){    
    // TODO: This could be optimized into one or two memcpy ops and pointer adjust
    int indx;
    for ( indx=0 ; indx < count ; indx++){
        StoreNumber(ds, values[indx]);
    }
       
    return 0;
    
}

int RetriveNumbers( struct dataStore* ds, int* values, int count ){
    // TODO: This could be optimized into one or two memcpy ops and pointer adjust
    int rdIndx;
    for(  rdIndx = 0; !IsStoreEmpty(ds) && rdIndx < count; rdIndx++ ){
     RetriveNumber(ds, values+rdIndx);  
    }
    
    return rdIndx;
    
}