/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   movingAverageFilter.h
 * Author: pooh
 *
 * Created on April 22, 2018, 11:20 AM
 */

#ifndef MOVINGAVERAGEFILTER_H
#define MOVINGAVERAGEFILTER_H

#include "dataStore.h"


struct movingAverageFilter{
    struct dataStore* ds;
    int* wrkBuff;
    int buffSize;
    
};


struct movingAverageFilter* CreateMovAvgFilter(int bufferSize);
void FreeMovAvgFilter(struct movingAverageFilter* mav);

int GetMovAvgValue( struct movingAverageFilter* mav, int value);

int DoMovAvgOnValues( struct movingAverageFilter* mav, int* values, int size);

#endif /* MOVINGAVERAGEFILTER_H */

