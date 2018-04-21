/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */




#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include <stdbool.h>

#include "strNumConv.h"
#include "dataStore.h"


void assert_equal( int actual, int expected, char*msg){
    if( actual != expected ) printf("ERROR!!! %s; expected(%d) recieved(%d)\n", msg, expected, actual);
}

void assert_str_equal( char* actual, char* expected, char*msg){
    if( strcmp(actual, expected) != 0 ) printf("ERROR!!! %s; expected(%s) recieved(%s)\n", msg, expected, actual);
}

void assert_equal_memcmp(void* actual, void* expected, size_t size, char*msg ){
    if( memcmp(actual, expected, size) != 0) {
               
        printf("ERROR!!! %s; actual(", msg );  
        for( int* start = (int*)actual; start < (int*)(actual+size) ; start++){
                printf("%d,", *start );   
        }
        printf(")\n");

    }
}

void test_converters (){
       
    char *p;
    int data[100];
    char outStr[100];
    int result;
    
    p = "5 17 183 2073 0 1 888 921 032 0 666 22 33 44 55  678";
    result = ConvertToIntArray(p, data, 100);
    
    assert_equal( result, 16, "E1:Wrong number of integers parsed"); 
            
    assert_equal( data[0], 5, "E2:Wrong value returned from ConvertToIntArray for index = 0");        
    assert_equal( data[8], 32, "E2a:Wrong value returned from ConvertToIntArray for index = 8");   
    assert_equal( data[15], 678, "E2b:Wrong value returned from ConvertToIntArray for index = 15");   
              
    result = ConvertToString(data, result, outStr, 30);
    
    assert_equal( result, 10, "E3a:Wrong number of ints returned from ConvertToString ");   

    assert_str_equal(outStr, "5 17 183 2073 0 1 888 921 32 0", "E4: Wrong string returned from ConvertToString" );
    
    result = ConvertToString(data, 16, outStr, 100);
    
    assert_equal( result, 16, "E3b:Wrong number of ints returned from ConvertToString ");   

    assert_str_equal(outStr, "5 17 183 2073 0 1 888 921 32 0 666 22 33 44 55 678", "E4: Wrong string returned from ConvertToString" );
 
}

void test_converter_badData(){
    
     char *p;
    int data[100];
    char outStr[100];
    int result;
    
    p = "5 17 usagi  1073 2 12abc 888 92 z032 0 a b, 696 212 333 444 535  68 12";
    result = ConvertToIntArray(p, data, 100);
    
    assert_equal( result, 16, "G1:Wrong number of integers parsed"); 
    
    int expected[] = {5,17,1073,2,12,888,92,32,0,696,212,333,444,535,68,12};
    assert_equal_memcmp(data, expected , sizeof(int[16]), "G2:wrong value returned from RetriveNumbers ");

    // HACK: this test is failing and correcting the issue will require 
    // rewriting ConvertToIntArray to not use scanf...  Left the test as  a reminder
    p = "123, 234, 345, 456,567,678,789,890";
    result = ConvertToIntArray(p, data, 100);
    
    assert_equal( result, 8, "G3:Wrong number of integers parsed"); 
    
    int expected2[] = {123,234,345,456,567,678,789,890};
    assert_equal_memcmp(data, expected2 , sizeof(int[8]), "G4:wrong value returned from RetriveNumbers ");

    
   
  
    
}

void test_dataStore(){
    struct dataStore* ds;
    int indx, result, out;
    
    ds = CreateDataStore(10);
    
    // add 10 entries
    for( indx=0 ; indx < 10 ; indx++ ){
        result = StoreNumber(ds, indx);
        assert_equal(result, 0, "D1:wrong result returned from StoreNumber ");
    }
     
    // read back 10
    for( indx=0 ; indx < 10 ; indx++ ){
        result = RetriveNumber(ds, &out);
        assert_equal(result, 0, "D2a:wrong result returned from RetriveNumber ");
        assert_equal(out, indx, "D2b:wrong value returned from RetriveNumber ");
    }
    
    // check for empty 
    result = RetriveNumber(ds, &out);
    assert_equal(result, -1, "D2b:wrong result returned from RetriveNumber ");  
    
    // Add 8 entries
    for( indx=0 ; indx < 8 ; indx++ ){
        result = StoreNumber(ds, indx);
        assert_equal(result, 0, "D3:wrong result returned from StoreNumber ");
    }
     
    // read 4 back 
    for( indx=0 ; indx < 4 ; indx++ ){
        result = RetriveNumber(ds, &out);
        assert_equal(result, 0, "D4a:wrong result returned from RetriveNumber ");
        assert_equal(out, indx, "D4b:wrong value returned from RetriveNumber ");
    }
    
    // 6 more   ( that's 10 into 10 byte buffer so we should be cool )
    for( indx=8 ; indx < 14 ; indx++ ){
        result = StoreNumber(ds, indx);
        assert_equal(result, 0, "D5:wrong result returned from StoreNumber ");
    }
       
    for( indx=4 ; indx < 14 ; indx++ ){
        result = RetriveNumber(ds, &out);
        assert_equal(result, 0, "D6a:wrong result returned from RetriveNumber ");
        assert_equal(out, indx, "D6b:wrong value returned from RetriveNumber ");
    }

    
    result = RetriveNumber(ds, &out);
    assert_equal(result, -1, "D7b:wrong result returned from RetriveNumber "); 
    
    // 11 more   ( that's 11 into 10 byte buffer so we should loose 1 )
    for( indx=0 ; indx < 11 ; indx++ ){
        result = StoreNumber(ds, indx);
        assert_equal(result, 0, "D8:wrong result returned from StoreNumber ");
    }
       
    for( indx=1 ; indx < 11 ; indx++ ){
        result = RetriveNumber(ds, &out);
        assert_equal(result, 0, "D9a:wrong result returned from RetriveNumber ");
        assert_equal(out, indx, "D9b:wrong value returned from RetriveNumber ");
    }
    
    
    result = RetriveNumber(ds, &out);
    assert_equal(result, -1, "D10b:wrong result returned from RetriveNumber "); 
  
    
    
}

void test_dataStore_bufferAccess(){
    
    struct dataStore* ds;
    
    int outBuff[20];
    
    int  result;
    
    ds = CreateDataStore(10);
    
    int inBuff[] = {0,11,22,33,44,555,666,77,88,99};
    // add 10 entries
    result = StoreNumbers(ds, inBuff, 10);
    assert_equal(result, 0, "F1:wrong result returned from StoreNumber ");
   
    
    result = RetriveNumbers(ds, outBuff, 20);
    assert_equal(result, 10, "F2a:wrong result returned from RetriveNumber ");
    assert_equal_memcmp(outBuff, inBuff , sizeof(int[10]), "F2b:wrong value returned from RetriveNumbers ");
    
    
    int inBuff2[] = {0,11,22,33,44,55,66,77,88,99, 123, 456, 789, 69, 96};
    int expected[] = {55,66,77,88,99, 123, 456, 789, 69, 96};
    // add 10 entries
    result = StoreNumbers(ds, inBuff2, 15);
    assert_equal(result, 0, "F3:wrong result returned from StoreNumber ");
   
    
    result = RetriveNumbers(ds, outBuff, 20);
    assert_equal(result, 10, "F4a:wrong result returned from RetriveNumber ");
    assert_equal_memcmp(outBuff, expected , sizeof(int[10]), "F4b:wrong value returned from RetriveNumbers ");

    

    
    
}


int main(){
    
    printf("Oh, bother...\n");

    test_converters();
    
    test_converter_badData();
            
    test_dataStore();
    
    test_dataStore_bufferAccess();
    
    printf("No bother...\n");
    
}