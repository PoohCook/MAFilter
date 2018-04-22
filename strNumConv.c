/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef TEST_COMPILE

#include <linux/init.h>
#include <linux/module.h> 
#include <linux/device.h> 
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ctype.h>

#else

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include <stdbool.h>
#include <ctype.h>

#include "testStubs.h"

#endif

#include "strNumConv.h"

#define NUMBER_TOO_LONG -2


int scan_for_int ( const char* scanp, int*valuep,  int* numScanned ){
    
    int buffSize = 6;
    char buffer[buffSize+1];
    
    char *buffp = buffer;
    *buffp = 0;
    *numScanned = 0;
    
    // advance  to next digit 
    while( *scanp != 0 &&  !isdigit(*scanp)) {
        scanp++;
        (*numScanned)++;
    }
    
    // scan digits into buffer 
    while( *scanp != 0 && buffp < (buffer+buffSize) &&  isdigit(*scanp)){
        *buffp++ = *scanp++;
        *buffp = 0;    //terminate the buffer
        (*numScanned)++;
    }
    
    //  number is longer that buffer
    if (isdigit(*scanp))return NUMBER_TOO_LONG;
    
    return kstrtoint (	buffer, 10, valuep);
    
    

}




int ConvertToIntArray(  const char* strIn, int* buffer_outp, int size ){
    
    
    const char* scanp = strIn;
    const char* scanEnd = scanp + strlen(strIn);
    int n;
    int indx=0;
    int result;
    
       
    while (scanp < scanEnd) {      
      result = scan_for_int(scanp,buffer_outp+indx, &n);
      if( result == 0)indx++;
      scanp += n;
      if( indx >= size)
          break;
    }
  
   
    return indx;
    
}

int ConvertToString ( int* buffer_inp, int buff_size, char* strOut, int str_size){
     
    char numBuffer[10];  
    int indx,result ;
    int strOutOffset = 0;
    
    for( indx=0 ; indx < buff_size ; indx++ ){
        result = sprintf(numBuffer, "%s%d", indx == 0 ? "" : " ", buffer_inp[indx] );
        if( result <= 0 )return -1; // trap bogus values..  though, can't think of what those might be 
        if( strOutOffset + result - 1 > str_size ) break; //quit if result will overflow output buffer
        
        strcpy(strOut+strOutOffset, numBuffer);
        strOutOffset += result;
    } 
    
    return indx;
}

