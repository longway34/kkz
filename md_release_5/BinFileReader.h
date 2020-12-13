/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BinFileReader.h
 * Author: longway
 *
 * Created on 7 июля 2016 г., 21:48
 */

#ifndef BINFILEREADER_H
#define BINFILEREADER_H

#include "../Common/CommonObjects.h"
#include "IReader.h"

class BinFileReader : public IReader {
protected:
    FILE *file;
    std::string filename;
    long fileSize;
    int16_t buffer;
    long countReadBytes;
public:
    BinFileReader();
    BinFileReader(const char* _fileName, CommonHeader *_cheader);
    virtual ~BinFileReader();

    virtual IReader *FileOpen(const char *_fileName); 
    virtual void *read();
    void stop(){};
    void start(){};
    inline virtual int getFileSize(){ return fileSize; }
    inline void StartStopInductor(bool on_off) {};
    
    private:

};

#endif /* BINFILEREADER_H */

