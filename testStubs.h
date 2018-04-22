/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   testStubs.h
 * Author: pooh
 *
 * Created on April 22, 2018, 11:32 AM
 */

#ifndef TESTSTUBS_H
#define TESTSTUBS_H

#define GFP_KERNEL 0
void *kmalloc(size_t size, int flags);

void kfree( void * blk);


int kstrtoint (	const char * s,
 	unsigned int base,
 	int * res);


#endif /* TESTSTUBS_H */

