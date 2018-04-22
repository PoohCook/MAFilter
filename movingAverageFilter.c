/**
 * @file   movingAverageFilter.c
 * @author Pooh Cook
 * @date   22 April 2018
 * @version 0.1
 * @brief   a set of utilities to perform a moving average of int array data 
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

#include "movingAverageFilter.h"

// utility to perform integer division rounded to nearest up or down value
// .5 or greater round up, below rounds down...  Tested to work on negative and 
// positive results
long rounded_int_divide ( long divided, int divisor ){
    int quotient, remainder, adjustment;
    
    quotient = divided/divisor;
    adjustment = (quotient > 0)? 1 : -1;
    
    remainder = ((divided*10*adjustment) / divisor) % 10;
    if( remainder < 5) adjustment = 0;  
    
    return quotient + adjustment;    
}

/** @brief This function is used to create a new movingAverageFilter of the specified size
 *          CAUTION: This functions allocates memory so make sure that FreeMovAvgFilter()
 *          Gets called in concert  
 *  
 *  @param buffer_size  size of the storage buffer to allocate
 *  @return pointer to new movingAverageFilter
 */
struct movingAverageFilter* CreateMovAvgFilter(int buffer_size){
    
    struct movingAverageFilter* mav = kmalloc(sizeof(struct movingAverageFilter), GFP_KERNEL);
    
    mav->ds = CreateDataStore(buffer_size);
    mav->wrkBuff = kmalloc(sizeof(int[buffer_size]), GFP_KERNEL);
    mav->buffSize = buffer_size;
       
    return mav;    
}

/** @brief This function is used free an allocated movingAverageFilter created by CreateMovAvgFilter
 *  
 *  @param mav   pointer to movingAverageFilter structure
 */
void FreeMovAvgFilter(struct movingAverageFilter* mav){  
    
    FreeDataStore(mav->ds);
    kfree(mav->wrkBuff);
    kfree(mav);
}

/** @brief This function is used to get a single averaged value from a movingAverageFilter.
 *         The value is added to the head of the store and then all value in the store are averaged 
 *         If the averaged store is already full then data from the tail will be lost 
 *  
 *  @param mav   pointer to movingAverageFilter structure
 *  @param value new value into the average
 *  @return new average value of the filter contents
 */
int GetMovAvgValue( struct movingAverageFilter* mav, int value){
    
    int count, indx;
    long tally = 0;
    
    StoreNumber(mav->ds, value);
    
    count = PeekBuffer(mav->ds, mav->wrkBuff, mav->buffSize);
    for (indx=0; indx < count ; indx++){
       tally += mav->wrkBuff[indx]; 
    }
  
    return rounded_int_divide(tally, count);
}

/** @brief This function is used to perform a moving average on a series of integers
 *         in a integer array based upon the movingAverageFilter. The averaging is based not 
 *         only upon the array values but also upon the current values stored in the movingAverageFilter
 *           
 *  @param mav    pointer to movingAverageFilter structure
 *  @param values pointer to an array of integer values to be modified
 *  @param size   count of integers in the values array
 *  @return  always 0 (success)
 */
int DoMovAvgOnValues( struct movingAverageFilter* mav, int* values, int size){
    int indx;
    for (indx = 0; indx < size ; indx++){
        values[indx]= GetMovAvgValue(mav, values[indx]);
    }
    return 0;
}
