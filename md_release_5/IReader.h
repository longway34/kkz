/**
 * @file 
 * File:   IReader.h
 * Author: longway
 *
 * Created on 1 июля 2016 г., 16:37
 *@brief Файл структур данных, касающихся источников данных
 */

#ifndef IREADER_H
#define	IREADER_H

#include "../Common/CommonObjects.h"

#define STATE_READER_EOF    0x0001      ///< Состояние Конец файла
#define DEF_SAVE_DUMP_FILE_NAME     DEF_READER_FILE_PATH"dump.bin"  ///< Имя файла данных ДМА
#define DEF_SAVE_SDUMP_FILE_NAME    DEF_READER_FILE_PATH"sdump.bin" ///< Имя файла данных сумм и отношений

#define READER_IS_INIT      0x0002  ///< Состояние буфер данных инициализирован
#define READER_IS_STOPPED   0x0004  ///< Состояние источник остановлен

#define READER_IS_STORED_COUNT_DATA  0x0020 //< Команда остановки инкремента счетчика байт записи

#define DEF_SAMPLES_IN_PACKETS      250
#define MAX_PACKET_QUERY_SIZE       8

#define inet_addr(a,b,c,d)	( ((uint32_t)a) | ((uint32_t)b << 8) | ((uint32_t)c << 16) | ((uint32_t)d << 24) )

#define DEF_MY_PORT                 8000
#define DEF_XMEGA_ADDR              inet_addr(192,168,2,106)
#define DEF_XMEGA_PORT              4001

#define ERR_PACKET_COUNT            0x01
#define ERR_NULL_SUMM_FOUND         0x02

//typedef struct _kkz_data {
//	uint32_t state;
//	uint32_t count;
//	uint32_t summ[MAX_SUMMS_NUMBER];
//	float ratio[MAX_RATIOS_NUMBER];
//	int16_t dump[DEF_SAMPLES_IN_PACKETS];
//} kkz_data_t;

typedef struct{
	uint32_t    state;					/* версия и возможные ошибки */
	uint32_t    count;					/* порядковый номер пакета */
	uint32_t	length;					/* длина поля данных в самплах*/
	uint32_t    summ[MAX_SUMMS_NUMBER];		/* суммы */
	float       ratio[MAX_RATIOS_NUMBER];		/* отношения */
	float		avrRatio[MAX_RATIOS_NUMBER];	/* фильтрованные отношения */
	uint16_t    dump[DEF_SAMPLES_IN_PACKETS];	/* данные АЦП */
} kkz_data_t;

#define UDP_STORE_BUFFER_SIZE       (250 * 2 * 1000)    //< длина буаука - последг=ние 1000 отсчетов

/**
 @brief Виртуальный класс - общий для всех источников данных
 * 
 * В классах потомках переопределить: метод \code void read(); \endcode;
 * определить значение свойств: elementSize, typeReader
 * 
 */
class IReader : public CommonObject{
protected:
    SettingsData *settings;         ///< Структура настройки \ref SettingsData
    SettingsData *_setts;           ///< Структура настройки по умолчанию \ref SettingsData
    void *dmaData;                   ///< буфер для чтения и манируляции 
    volatile unsigned long currentBufferSize; ///< текущий размер буфера в прочитанных элементах
    volatile unsigned long cursorPosition;    ///< текущая позиция курсора в буфере
    int elementSize;                 ///< размер элемента в байтах
    uint32_t addCursorPosition();    ///< Сдвиг позиции курсора внутри буфера
    int typeReader;                 ///< Тип источника
    int markerCount;                
    uint32_t countBytesStoredData;
    
    void findBegin();           ///< не реализовано
public:
    IReader();      //< конструктор по умолчанию
    /*@brief    конструктор
     \param _sets набор установок из рецепта \ref SettingsData*/
    IReader(SettingsData *_sets); 
    virtual ~IReader();
    
    virtual void *read() =  0;          ///< чтение очередной порции данных
    virtual int getTypeReader() {return typeReader;}    ///< получение источника см \ref _TYPE_READER
    void *const getNextElement();       ///< получение следующего элемента буфера данных
    virtual void start(){};           ///< старт объекта получения данных. Реализуется в потомках
    virtual void stop(){};            ///< оствнов объекта получения данных. Реализуется в потомках
    virtual void pause(bool on_off){};  ///< не используется
    virtual uint32_t getErrorReady(){ return STATE_OK;} ///< получить состояние объекта получения данных. 0 (STATE_OK); иначе код ошибки
                                                   // в потомках перепереопределить под конкретный класс
    inline virtual int getFileSize(){ return -1; }
    inline virtual void StartStopInductor(bool on_off) = 0;
    void setMark(uint count){ markerCount = count; }
//    virtual void setIncrementCountStoredData(bool on_off) {if(on_off) setState(READER_IS_STORED_COUNT_DATA); 
//                                                            else clearState(READER_IS_STORED_COUNT_DATA);
//    }
private:

};

#endif	/* IREADER_H */

