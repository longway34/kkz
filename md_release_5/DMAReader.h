/* 
 * File:   DMAReader.h
 * Author: longway
 *
 * Created on 1 июля 2016 г., 16:59
 */

#ifndef DMAREADER_H
#define	DMAREADER_H

#include "IReader.h"
#include "DM_7520.h"

#define DEF_DMA_STORE_BUFFERS_NUMBER    175

class DMAStoreBuffer : CommonObject{
    void *buffer;
    void *storeBuffer;
    long bufferSize; //< Длина в буфера байтах
    int buffersNumber; //< Кол-во хранимых буферов 
    int size, lastIndex, firstIndex;
    void *lastRpt;
    
public:
    DMAStoreBuffer(long _bufferSize = DEF_DMA_BUFFER_SIZE, int _buffersNumber = DEF_DMA_STORE_BUFFERS_NUMBER){
        bufferSize = _bufferSize; buffersNumber = _buffersNumber;
        buffer = NULL;
        storeBuffer = NULL;
        buffer = malloc(bufferSize * buffersNumber);
        storeBuffer = malloc(bufferSize * buffersNumber);
        
        size = 0; lastIndex = 0; firstIndex = 0;
        lastRpt = buffer;
    }
    
    virtual ~DMAStoreBuffer(){
        if(buffer) { free(buffer); buffer = NULL;}
        if(storeBuffer) {free(storeBuffer); storeBuffer = NULL;}
    }
    int getSize(){ return size; }
    
    void *add(void *_buffer){
//        if(size < buffersNumber) {
//            size++;
//        } else {
//            firstIndex++; if(firstIndex == buffersNumber) firstIndex = 0;
//        }
//        
//        lastIndex++; if(lastIndex == buffersNumber) lastIndex = 0;
//        void *rpt = buffer + (lastIndex * bufferSize);
//        lastRpt = memcpy(rpt, _buffer, bufferSize);

        memmove(buffer, buffer+bufferSize, bufferSize * (buffersNumber - 1));
        lastRpt = memmove(buffer + bufferSize * (buffersNumber - 1), _buffer, bufferSize);

        return lastRpt;
    }
    
    void *getBuffer(int index = 0){
        int num=firstIndex, count = 0;
        while(count++ < index){
            num++; if(num >= size) num = 0;
        }
        void *res = buffer + (num * bufferSize);
        return res;
    }
    
    long getLenBuffer(){
        return buffersNumber * bufferSize;
    }
    void *getStoredBuffer(){
//        void *rpt1 = buffer + (firstIndex * bufferSize); 
//        long len1 = (size - firstIndex - 1) * bufferSize;
//        memcpy(storeBuffer, rpt1, len1);
//        void *src1 = storeBuffer + len1; 
//        long len2 = (firstIndex) * bufferSize;
//        memcpy(src1, buffer, len2);

//        void *dst2 = storeBuffer; 
//        int num = firstIndex; 
//        int count = 0;
//        void *src2 = buffer + (firstIndex * bufferSize);
//        while(count++ < size){
//            memcpy(dst2, src2, bufferSize);
//            num++; if(num >= buffersNumber) num = 0;
//            dst2 = dst2 + bufferSize;
//            if(num == 0){
//                src2 = buffer;
//            } else {
//                src2 = src2 + bufferSize;
//            }
//        }
//        return storeBuffer;
        memcpy(storeBuffer, buffer, bufferSize * buffersNumber);
        return storeBuffer;
    }

};

class DMAReader : public IReader{
    Dma *dma;       // указатель на ДМА
//    int markerCount;
    DMAStoreBuffer *stored;
public:
    DMAReader(SettingsData *setts);
    virtual ~DMAReader();
    virtual void *read();
    virtual void start();
    virtual void stop();
    virtual void pause(bool on_off);

    virtual void StartStopInductor(bool on_off = true);
    virtual uint32_t setStoredData(bool on_off, int type = 0);
    
    
private:

};

#endif	/* DMAREADER_H */

