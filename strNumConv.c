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

#else

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include <stdbool.h>

#endif

#include "strNumConv.h"


int ConvertToIntArray( const char* strIn, int* buffer_outp, int size ){
    
    const char* scanp = strIn;
    int n;
    int indx=0;
    
    while (sscanf(scanp, "%d%n", buffer_outp+indx, &n) == 1) {
      indx++;
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

