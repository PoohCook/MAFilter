/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dataStore.h
 * Author: pooh
 *
 * Created on April 20, 2018, 4:42 PM
 */

#ifndef DATASTORE_H
#define DATASTORE_H



struct dataStore {
    int* buffer;                        ///< Data buffer pointer
    int buff_size;                      ///< Size of buffer
    int* buff_inp;                      ///< input pointer
    int* buff_outp;                     ///< output pointer
};

struct dataStore* CreateDataStore(int bufferSize);
void FreeDataStore(struct dataStore* ds);
int StoreNumber( struct dataStore* ds, int value );
int RetriveNumber( struct dataStore* ds, int* outValue );
bool IsStoreEmpty( struct dataStore* ds);
int StoreNumbers( struct dataStore* ds, int* values, int count );
int RetriveNumbers( struct dataStore* ds, int* values, int count );

#endif /* DATASTORE_H */

