/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#ifndef TEST_COMPILE

#include <linux/module.h> 
#include <linux/slab.h>


#else

#include <stdlib.h>
#include <stdbool.h>
#include "testStubs.h"


#endif

#include "movingAverageFilter.h"



struct movingAverageFilter* CreateMovAvgFilter(int bufferSize){
    
    struct movingAverageFilter* mav = kmalloc(sizeof(struct movingAverageFilter), GFP_KERNEL);
    
    mav->ds = CreateDataStore(bufferSize);
    mav->wrkBuff = kmalloc(sizeof(int[bufferSize]), GFP_KERNEL);
    mav->buffSize = bufferSize;
       
    return mav;    
}

void FreeMovAvgFilter(struct movingAverageFilter* mav){  
    
    FreeDataStore(mav->ds);
    kfree(mav->wrkBuff);
    kfree(mav);
}

    
long rounded_int_divide ( long divided, int divisor ){
    int quotient, remainder, adjustment;
    
    quotient = divided/divisor;
    adjustment = (quotient > 0)? 1 : -1;
    
    remainder = ((divided*10*adjustment) / divisor) % 10;
    if( remainder < 5) adjustment = 0;  
    
    return quotient + adjustment;    
}

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


int DoMovAvgOnValues( struct movingAverageFilter* mav, int* values, int size){
    int indx;
    for (indx = 0; indx < size ; indx++){
        values[indx]= GetMovAvgValue(mav, values[indx]);
    }
    return 0;
}
