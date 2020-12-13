/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CombinedSummary.h
 * Author: longway
 *
 * Created on 28 мая 2017 г., 21:38
 */

#ifndef COMBINEDSUMMARY_H
#define COMBINEDSUMMARY_H

#include "Recepts.h"
#include "IReader.h"
#include "Summary.h"

class CombinedSummary : CommonObject{
    int SummaryCount;
    std::vector<Summary*> summares;
    std::vector<IReader*> readers;
public:
    CombinedSummary();
    CombinedSummary(Recepts *settings);
    CombinedSummary(const CombinedSummary& orig);
    virtual ~CombinedSummary();
private:

};

#endif /* COMBINEDSUMMARY_H */

