/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BinFileReader.cpp
 * Author: longway
 * 
 * Created on 7 июля 2016 г., 21:48
 */

#include "BinFileReader.h"

BinFileReader::BinFileReader(){
}

BinFileReader::BinFileReader(const char *_fileName, CommonHeader *_cheader){
    dmaData = NULL; file = NULL;
    if(_cheader){
        memcpy(cheader.data, _cheader->data, sizeof(CommonHeader));
    }
    elementSize = (MAX_CHANNELS_NUMBER * 2) * 100;
    dmaData = (int16_t*)malloc(elementSize);
    typeReader = TYPE_READER_BIN;
    
    currentBufferSize = 0;
    countReadBytes = 0;
    
    if(!FileOpen(_fileName)){
        fprintf(stdout,"Error!!! File Not Openning %s\n", filename.c_str());
    }
    addState(STATE_READER_EOF);
}

BinFileReader::~BinFileReader(){
    if(file){
        fclose(file);
        file = NULL;
    }
    if(dmaData){
        free(dmaData);
        dmaData = NULL;
    }
}

IReader *BinFileReader::FileOpen(const char *_fileName)
{
    currentBufferSize = 0;
    
    filename = _fileName;
    if(filename.at(0) != '/' && filename.at(0) != '.') filename = DEF_READER_FILE_PATH+filename;
    file = fopen(filename.c_str(),"rb");
    if(file) {
        cursorPosition = 0;
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        return this;
    } else {
        return NULL;
    }
}

void *BinFileReader::read()
{
    bool ready = false; currentBufferSize = 0;
    long maxCountBytes = fileSize / (elementSize * 100) * cheader.channelsNumber;
    maxCountBytes *= (elementSize * 100) * cheader.channelsNumber;
    
    if(file){
        while(!ready){
            if(feof(file)){ // файл закончился
                this->addState(READER_IS_STOPPED);
                fseek(file,0,SEEK_SET);
                countReadBytes = 0;
            }
            currentBufferSize = fread(dmaData, 1, elementSize, file);
            
            if(currentBufferSize < 0) {               
                break;
            } else if(currentBufferSize == elementSize) {
                if(getTypeReader() == TYPE_READER_BIN){
                    currentBufferSize = currentBufferSize / 2;
                } else {
                    currentBufferSize = currentBufferSize / elementSize;
                }
                countReadBytes += elementSize;
                ready = true;
            }
        }
        if(ready){
            return dmaData;
        } else {
            return NULL;
        }
    } else { // файл не открыт
        return NULL;
    }
    return dmaData;
}
