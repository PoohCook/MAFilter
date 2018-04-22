/**
 * @file   strNumConv.c
 * @author Pooh Cook
 * @date   20 April 2018
 * @version 0.1
 * @brief   a set of utilities to convert strings of numbers to and from arrays of integers 
 */

// these utilities are intended for use in a Linux Kernel Module but are unit tested in a 
// user mode program. The TEST_COMPILE switch is used to determine which mode to compile into
#ifndef TEST_COMPILE

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ctype.h>

#else

#include<stdio.h>
#include<string.h>
#include <stdbool.h>
#include <ctype.h>

#include "testStubs.h"      // Stubs out kerel mode utilites for test operation

#endif

#include "strNumConv.h"

#define NUMBER_TOO_LONG -2

// hand rolled string scan for number because scanf is soooo useless
static int scan_for_int ( const char* scanp, int*valuep,  int* numScanned ){
    
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


/** @brief This function is used to convert an string to an int buffer array 
 *  
 *  @param strIn  string to be converted. Assumes a base 10 radix series of integers 
 *                seperated by commas, spaces, semicolons or any non digit char
 *  @param buffer pointer to buffer where ints are to be written 
 *  @param size   number of ints in the buffer
 *  @return  number of ints that were parsed 
 */
int ConvertToIntArray(  const char* strIn, int* buffer, int size ){
    // TODO This should really take a radix so hex formated ints can be supported
    
    const char* scanp = strIn;
    const char* scanEnd = scanp + strlen(strIn);
    int charScanCount;
    int indx=0;
    int result;
       
    while (scanp < scanEnd) {      
      result = scan_for_int(scanp,buffer+indx, &charScanCount);
      if( result == 0)indx++;
      scanp += charScanCount;
      if( indx >= size)
          break;
    }
   
    return indx;
    
}

/** @brief This function is used to convert an int buffer array to string of comma delimited 
 *    radix 10 encoded numbers
 *  
 *  @param buffer     pointer to the array of ints being read from
 *  @param buff_size  number of ints in the buffer
 *  @param str        pointer to charater buffer where the string is to be written 
 *  @param str_size   size of the character buffer
 *  @return number of ints that were encoded
 */int ConvertToString ( int* buffer, int buff_size, char* str, size_t str_size){
     
    char numBuffer[10];  // temp buffer to write each number to
    int indx,result ;
    int strOutOffset = 0;
    
    for( indx=0 ; indx < buff_size ; indx++ ){
        result = sprintf(numBuffer, "%s%d", indx == 0 ? "" : " ", buffer[indx] );
        if( result <= 0 )return -1; // trap bogus values..  though, can't think of what those might be 
        if( strOutOffset + result + 1 > str_size ) break; //quit if result will overflow output buffer
        
        strcpy(str+strOutOffset, numBuffer);
        strOutOffset += result;
        
    } 
    
    str[strOutOffset]= 0;   //null terminate the string 
    
    return indx;
}

