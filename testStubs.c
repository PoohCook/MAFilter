/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include "testStubs.h"


void *kmalloc(size_t size, int flags){
  return malloc(size);   
}

void kfree( void * blk){
    free(blk);
}

int kstrtoint (	const char * s,
 	unsigned int base,
 	int * res)
{
    *res =  atoi(s);
    return 0;
}