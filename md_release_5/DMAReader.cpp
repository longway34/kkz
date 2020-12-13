/* 
 * File:   DMAReader.cpp
 * Author: longway
 * 
 * Created on 1 июля 2016 г., 16:59
 */

#include "DMAReader.h"
#include "Parsers.h"

extern bool noConsole;
extern bool wrEvents;
extern ILogger *logger;

DMAReader::DMAReader(SettingsData *setts) {
    settings = setts;
    dma = new Dma(settings);
    currentBufferSize = DEF_DMA_BUFFER_SIZE / sizeof(int16_t);
    elementSize = DEF_ELEMENT_SIZE;
    typeReader = TYPE_READER_DMA;
    addState(READER_IS_STOPPED);
    stored = new DMAStoreBuffer();
}

DMAReader::~DMAReader() {
    dma->Stop();
    delete stored;
    delete dma;
}

void *DMAReader::read(){    
    void *res = NULL;
    while(1){
        res = dma->Read();
        if(dma->state != STATE_OK){
            addState(ERR_READER_QUERY);
        } else {
            clearState(ERR_READER_QUERY);
        }
        if(res){
            dmaData = res;
        } else { 
#ifdef DEBUG
            if(!noConsole){
                fprintf(stdout, "Error reading in DMAReader...\n");
            }
#endif
        }
//        if(isStoredData()){
///******************************** Отладка 05,02,2017 ********************************************/
//            if(isSetState(READER_IS_STORED_COUNT_DATA)){ // обычный режим записи с увеличением счетчика и проверкой верхней возножной границей записи
//                countBytesStoredData += DEF_DMA_BUFFER_SIZE;
//                if(countBytesStoredData >= 10000000){
//                    countBytesStoredData = 0; // начинаем писать по новой
//                    cancelSaveData(fd);
//                    setStoredData(false);
//                    setStoredData(true);
//                    addState(READER_IS_STORED_COUNT_DATA);
//                }
//            } else {
//                markerCount = 1;
//            }
///******************************** Отладка 05,02,2017 ********************************************/
//        }
        if(markerCount > 0){
            for(uint i=0; i < 2000; i++){
                ((uint16_t*)(dmaData))[i] &= 0xFFF9;
            }
            markerCount--;
        }
        if(wrEvents){
            stored->add(dmaData);
//            saveData(fd, dmaData, DEF_DMA_BUFFER_SIZE);
        }
        break;
    }
    return res;
};

void DMAReader::pause(bool on_off){
    if(on_off){
        if(!isSetState(READER_IS_STOPPED)){
            dma->Pause(on_off);
            addState(READER_IS_STOPPED);
        }
    } else {
        if(isSetState(READER_IS_STOPPED)){
            dma->Pause(on_off);
            clearState(READER_IS_STOPPED);
        }
    }
}

void DMAReader::start(){    
    dma->Start();
    clearState(READER_IS_STOPPED);
//    dma->StartStopInductor(true);
};

void DMAReader::stop(){    
    dma->Stop();
//    dma->StartStopInductor(false);
    addState(READER_IS_STOPPED);
};

void DMAReader::StartStopInductor(bool on_off){
   dma->StartStopInductor(on_off); 
}

uint32_t DMAReader::setStoredData(bool on_off, int type) {
    if(on_off){
        fname = DEF_READER_FILE_PATH"usb/";
        fname += "dump"+Helper::toString(type)+".bin";

        if(fd > 0){
            fprintf(stdout, "\nErr: Dump file %s is allready opened...", fname.c_str());
            storedData = true;
            return ERR_READER_STORE_DATA;
        }
        fd = open64(fname.c_str(), O_CREAT | O_WRONLY | O_NONBLOCK | O_TRUNC | O_APPEND,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(fd < 0){
            storedData = false;
            fprintf(stdout, "\nErr: Dump file %s is not open...\n", fname.c_str());
            return ERR_READER_STORE_DATA;
        } else {
            storedData = true;
            saveData(fd, stored->getStoredBuffer(), -1 * stored->getLenBuffer());
            
#ifdef DEBUG
            if(!noConsole)
                fprintf(stdout, "\nDump file %s is open...\n", fname.c_str());
#endif

            return STATE_OK;
        }
    } else {
        if(fd > 0){
#ifdef DEBUG
            if(!noConsole)
                fprintf(stdout, "\nDump file %s is closed...\n", fname.c_str());
#endif
            if(logger){
                logger->makeSyslogMsg("stored "+fname);
            }
            storedData = false;
            close(fd);
            fd = -1;
        }
        return STATE_OK;
    }
}

//uint32_t DMAReader::setStoredData(bool on_off, int type) {
//    if(on_off){
//        fname = DEF_READER_FILE_PATH;
//        fname += "dump"+Helper::toString(abs(type))+".bin";
//
//            if(fd > 0){
//            close(fd);
//        }
//        if(type >= 0){
//            fd = open64(fname.c_str(), O_CREAT | O_WRONLY | O_NONBLOCK | O_TRUNC | O_APPEND,
//                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//            clearState(STATE_SAVE_OPEN32);
//        } else {
//            fd = open(fname.c_str(), O_CREAT | O_WRONLY);
//            addState(STATE_SAVE_OPEN32);
//        }
//        if(fd < 0){
//            storedData = false;
//            if(!noConsole){
//                fprintf(stdout, "Err: Dump file %s is not open...", fname.c_str());
//            }
//            return ERR_READER_STORE_DATA;
//        } else {
//            storedData = true;
////            type = abs(type);
//#ifdef DEBUG
//            if(!noConsole){
//                fprintf(stdout, "Dump file %s is open...", fname.c_str());
//            }
//#endif
//            if(isSetState(STATE_SAVE_OPEN32)){
//                int count = 0; int index = dma->readingCount;
//                index++; if(index > DMA_MAX_QUEUE) index = 1;
//                index++; if(index > DMA_MAX_QUEUE) index = 1;
//#ifdef DEBUG
//        if(!noConsole){
//            fprintf(stdout, "Store dump file %s is open from %d as index...", fname.c_str(), index);
//        }
//#endif
//                while(count++ < DMA_MAX_QUEUE - 1){
//                    saveData(fd, dma->dmaUserBuffer + (DEF_DMA_BUFFER_SIZE * (index - 1)) / 2, DEF_DMA_BUFFER_SIZE);
//                }
////                storedData = false;
////                close(fd);
//            }
//
//            return STATE_OK;
//        }
//    } else {
//        if(fd > 0) close(fd); 
//        fd = -1;
//        storedData = false;
//        if(!noConsole){
//            fprintf(stdout, "Dump file %s is cloced...", fname.c_str());
//        }
//        clearState(STATE_SAVE_OPEN32);
//        return STATE_OK;
//    }
//}
