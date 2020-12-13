/**@file
 * @brief Файл реализации методов класса IReader 
 */

#include "IReader.h"

IReader::IReader():CommonObject() {
    dmaData = NULL;
    currentBufferSize = 0;
    cursorPosition = 0;
    elementSize = DEF_ELEMENT_SIZE;
    settings = new SettingsData();
    _setts = settings;
    addState(STATE_READER_EOF);
    markerCount = 0;
    countBytesStoredData = 0;
}

IReader::~IReader() {
   delete _setts; 
}

/*
 Сдвиг позиции курсора внутри буфера
 @return
 Возвращает текущее состояние
 */
uint32_t IReader::addCursorPosition(){
    cursorPosition++; 
    if(cursorPosition >= currentBufferSize) {
        addState(STATE_READER_EOF);
    }
    return getState();
}

/*
 * Получение следующего элемента
 * @return
 * Указатель на элемент буфера. NULL - ошибка чтения
*/
void *const IReader::getNextElement(){
    void *res = NULL;
    if(isSetState(STATE_READER_EOF)){
        dmaData = read();
        if(dmaData){
            clearState(STATE_READER_EOF);
            if(isError()){
#ifdef DEBUG
                fprintf(stdout, "..........stop state = %x\n", state);
#endif
            }
            cursorPosition = 0;
        } else {
#ifdef DEBUG
            fprintf(stdout,"Error reading in IReader dma = %p...\n", dmaData);
#endif
            addState(STATE_OBJECT_ERROR);
            return NULL;
        }
    }
    if(dmaData){
        if(typeReader == TYPE_READER_BIN){
            res = (dmaData) + cursorPosition * sizeof(int16_t);
        } else {
            res = (dmaData) + cursorPosition * elementSize;
        }
        addCursorPosition();
    }
    if(res == 0x0){
#ifdef DEBUG
        fprintf(stdout,"Error reading in IReader res = %p...\n", res);
#endif
    }
    return res;
}

