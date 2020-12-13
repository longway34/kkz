/**
 * @file    
 * @brief   Файл общих типов, настроек и значений по умолчанию 
 * 
 * File:   CommonObjects.h
 * Author: longway
 *
 * Created on 1 июля 2016 г., 14:39
 */

#ifndef COMMONOBJECTS_H
#define	COMMONOBJECTS_H

#include <fcntl.h>
#include <float.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>  //shm_open
#include <stdio.h>      //printf
#include <stdlib.h>     //exit
#include <unistd.h>     //close
#include <string.h>     //strerror
#include <iostream>
#include <sstream>
#include <fstream>
#include <aio.h>
#include <queue>
#include <mutex>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <syslog.h>
#include <sys/reboot.h>

#include <stdint.h>
#include <memory.h>
#include <vector>
#include <sys/shm.h>
#include <sys/mman.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <c++/5/cmath>
#include <typeinfo>
#include <signal.h>

#define SHR_OBJECT_NAME     "shm_key" //<   Имя файла-ключа расширенной области памяти
#define DEF_READER_FILE_PATH    "/home/longway/Common/" //< Каталог для файлов записи/чтения дампа DMA
#define DEF_BIN_FILE_READER_NAME    DEF_READER_FILE_PATH"dump.bin"; //< Имя файла рецепта
#define DEF_RECEPT_FILE_PATH    "/home/longway/Common/" //< Каталог файла рецепта
#define MAX_GRAPHIC_OBJECTS 500    //< максимальное колличество точек на графиках
#define MAX_CHANNELS_NUMBER 9       //< максимальное количество каналов
#define DMA_CHANNELS_NUMBER 8       //< максимальное количество каналов для DM-7520
#define DEF_CHANNELS_NUMBER 6       //< рабочее количество каналов
#define MAX_SUMMS_NUMBER    3       //< максимальное колличество сумм
#define MAX_RATIOS_NUMBER   2       //< максимальное колличество отношений
//#define DEF_PC_RATE_ON_CHANNEL  100000  // частота опроса АЦП для каждого канала, Гц
#define DEF_PC_RATE_ON_CHANNEL  100000  //< частота опроса АЦП для каждого канала, Гц
#define DEF_UTC_RATE            1000    //< частота работы ренератора, Гц
#define PERIOD_SAMPLES          (DEF_PC_RATE_ON_CHANNEL / DEF_UTC_RATE)     //< количество самплов в каждом периоде
#define HALF_PERIOD_SAMPLES     (PERIOD_SAMPLES / 2) //< количество самплов в каждом полупериоде

#define DEF_PASSWD          "qwerty" //< Пароль входа в режим настройки
#define DEF_AVERAGES_NUMBER     250                 //< суммирования при переходе на следующую итерацию (кратно половине периода 50 самплам)    
#define DEF_SHIFT_SIZE          91                  //< количесто сумм для сдвигового регистра фильтра отношений размер области

#define DEF_DMA_BUFFER_SIZE (64 * 1024) //< Буфер ДМА, размер в байтах
#define DEF_ELEMENT_SIZE    2 //< Размер элемента для экземпляров потомков класса IReader, если не указамо другое, в байтах

// определения отношений
#define S1_S0       0 // summ2/summ1        середина к началу (буты)
#define S2_S1       1 // summ3/summ2 main   конец к середине (материал)

static int dm75_chn[MAX_CHANNELS_NUMBER] = {0,1,1,2,3,4,4,5};// для 800000Гц{1, 1,  3, 3,  4, 4,  5,  5};
static int ps_chn[MAX_CHANNELS_NUMBER] =  {0,-1, 1, 2, 3, -1, 4, 5}; // для 800Гц {0, 1, -1, 2,  5, 4, -1,  3};

/**\brief Структура для передачи параметров обнаружения в качестве аргемента команды
 * формироуется объектом AParser при событии обнаркжения
 */
struct eventMessageData{
    int ch; ///< Битовая маска каналов, в которых произошло обнаружение.
    int length; ///< Максимальная длина зоны превышенияя порога
    float sigma; ///< Максимальное доспигнутое значение \f[$\sigma$\f];
    int countChange; ///< результат работы корректора ошибок аналогового тракта
    float R2;
};

#define TYPE_DATA_DMA           0 ///< исходный ДМА сигнал
#define TYPE_DATA_SUMMS         1 ///< полученные суммы
#define TYPE_DATA_AVR_RATIOS    2 ///< среднее значение отношений
#define TYPE_DATA_RATIOS        3 ///< полученные отношения
#define TYPE_DATA_DIFF_RATIO    11 ///< \deprecated разность ratios100[channel][s2/s1]-ratios100[channel][s1/s0] 
#define TYPE_DATA_DIFF         16 ///< \depreceted среднеквадратичное отклонение от % отношений без знака
#define TYPE_DATA_DIFF2         17 ///< \deprecated среднеквадратичное отклонение от % отношений со знаком
#define TYPE_DATA_MO       18 ///< \deprecated график поведения критерия от 09.12.2014
#define TYPE_DATA_SIGNAL_SIGMA       19 ///<\deprecated график поведения критерия от 09.12.2014
#define TYPE_DATA_R2        20  ///< график значений коэффициента детерминирования от 10.09.2016
#define TYPE_DATA_SIGMA    21  ///< график значений сигнала в "сигмах"
#define TYPE_DATA_F2P_SUMMS     22  ///<\deprecated график отфильтрованных сумм
#define TYPE_DATA_3_SIGMA       23  ///< график 3 среднеквадратичных отклонений
#define TYPE_DATA_LOG_SIGMA       24  ///< график 3 среднеквадратичных отклонений
//#define TYPE_DATA_RATIOS_100    4 // полученные отношения в процентах от мин/макс
//#define TYPE_DATA_FRONT_100     5 // фронт отношений в процентах от мин/макс
//#define TYPE_DATA_BACK_100      6 // срез отношений в процентах от мин/макс
//#define TYPE_DATA_AVR_RATIO     7 // среднее значение отношения
//#define TYPE_DATA_DIFF_FRONT    8 // разность ratiosFront100[channel][s2/s1]-ratiosFront100[channel][s1/s0]
//#define TYPE_DATA_FRONT         9 // фронт сигнала в абсолютных значениях
//#define TYPE_DATA_BACK          10 // срез сигнала в абсолютных значениях
//#define TYPE_DATA_INTEGRAL_FRONT 12 // интеграл разности front100 s2/s1 - s1/s1 и от
//#define TYPE_DATA_INTEGRAL_RATIO 13 // интеграл разности ratio100 s2/s1 - s1/s1 и от
//#define TYPE_DATA_CORREL_RATIO 14 // корелляция ratio100 s2/s1 - s1/s0
//#define TYPE_DATA_EMPTY_RATIOS 15 // отношения от калибровки пустой ленты

#define TYPE_NO_DATA            100

#define TYPE_READER_DMA     0   ///< Источник данных - АЦП DM-7520
#define TYPE_READER_BIN     1   ///< Источник данных - файл буферов ДМА
#define TYPE_READER_SUM     2   ///< Источник данных - файл суммарных значений
#define TYPE_READER_TEST    3   ///< Источник данных - преграммный генератор
#define TYPE_READER_UDP     4   ///< Источник данных - UDP клиент

#define SM_DATA_SIZE    512         // размер структуры данных в байтах
union sm_data{    ///< единица передоваемой информации между процессами и потоками
    char data[SM_DATA_SIZE];
    struct{
        unsigned long   summs[MAX_CHANNELS_NUMBER][MAX_SUMMS_NUMBER];       ///< суммы                    (4*8*3 = 96 bytes)
        float           ratios[MAX_CHANNELS_NUMBER][MAX_RATIOS_NUMBER];     ///< отношения                (4*8*2)= 64 bytes
        float           AvrRatio[MAX_CHANNELS_NUMBER][MAX_RATIOS_NUMBER];   ///< скользящее средние выход сумматора (4*8*2 = 64 bytes)
        float           signalSigma[MAX_CHANNELS_NUMBER][MAX_RATIOS_NUMBER];      ///< AvrRatios измеренный в сигмах (4*8*2) = 64 bytes
        float           sigma[MAX_CHANNELS_NUMBER][MAX_RATIOS_NUMBER];      ///< среднее квадратичное отклонение сумм  (4*8*2 = 64 bytes)
        float           MO[MAX_CHANNELS_NUMBER][MAX_RATIOS_NUMBER];         ///< мат. ожидание   (4*8*2 = 64 bytes)
        float           R2[MAX_CHANNELS_NUMBER];                        ///< коэффициент детерминации молели   (4*8 = 32 bytes)
        uint8_t         events[MAX_CHANNELS_NUMBER];                        ///< код метода определения сработки в канале 0 - нет сработки (1*8 = 8 bytes)
        uint8_t         hState;                                             ///< служебные (состояние оборудования) маркеры (1 bytes)
        uint8_t         sState;                                             ///< служебные (состояние оборудования) маркеры (1 bytes)
        uint8_t         evMask;
        unsigned long   time;                                               ///< время измерения (индекс для графики) (4 байта) = 463 байт
//        char            unused[128];
    };  // итого размер блока - 1024 bytes
};
typedef sm_data SummsRatiosData;

#define COMMON_HEADER_DATA_SIZE 32  // размер общего для всех обектов заголовка в байтах
union common_header{
    char data[COMMON_HEADER_DATA_SIZE];
    struct{
        int16_t channelsNumber;        ///< колиество рабочих каналов
        int16_t zonesNumber;           ///< количество рабочих зон
        int16_t ratiosNumber;          ///< количество рабочих отношений
    };
};
typedef common_header CommonHeader;

union Zone{
    uint16_t  data[2];
    struct
    {
        uint16_t minPointNumber;
        uint16_t maxPointNumber;
    };
};


#define DEF_P_SIGMA         10  ///< Порог превышения (по умолчанию)
#define DEF_P_LENGTH        50  ///< Длина превышения порога (по умолчанию)

#define DEF_MIN_R2          0.8     ///< порог значения коэффициента детерминации модели (по умолчанию)
#define DEF_ERR_LENGTH      1000    ///< порог длины общего превышения (по умолчанию)
#define DEF_AVR_SLOW        1000    ///< длина буфера т.н. длинного фильтра (по умолчанию)
#define DEF_PAR_LEN         10      ///< длина буфара модуля апроксимацци (по умолчанию)
#define DEF_PAR_MIN         10      ///< минимальная длина буферов объекта апроксимации для начала вычаслений. (по умолчанию)

#define DEF_CALIBRATE_MAX 5 ///< количество настренных наборов калибровки в рецепте (по умолчанию)
#define DEF_CALIBRATE_DEF   3   ///< начальный набор калибровки при запуске (по умолчанию)

#define DEF_K_DISP          0.9999  ///< коэффициент приращения дисперсии для объекта \ref SigmaSlide (по умолчанию)
#define DEF_K_MO            0.999   ///< коэффициент приращения мат. ожидания для объекта \ref SigmaSlide (по умолчанию)
#define DEF_NUM_SIGMA       3   ///< минимальное значение сигмы для начала процесса выработки решения (по умолчанию)

#define DEF_NUM_SIGMA_F     4   ///< минимальное значение сигмы для срабатывания фильтра обработки ошибок (по умолчанию)
#define DEF_LEN_SIGMA_F     32   ///< длина фильтра обработки ошибок (по умолчанию)

#define CALIBRATE_DATA_SIZE 8 ///< длина струкруры пороговых значений отдельного уровня чувствительности
/**\brief Струкрура пороговых значений отдельного уровня чувствительности
 */
union calibrate_data{
    char data[CALIBRATE_DATA_SIZE];
    struct{
        float pSigma;          ///< порог значения sigma             (4 байта)
        int pLength;          ///< максимальная длина для ошибки превышения kk (2 байта)
    };
};
typedef calibrate_data CalibrateData;

#define SETTINGS_DATA_SIZE 1024 ///< длина структуры екущих установочных данных
/**\brief Структура текущих установочных данных
 */
union settings_data{
    char data[SETTINGS_DATA_SIZE];
    struct{
        uint32_t averagesNumber;        ///< количество сумм, учавствующих во вторичном сдвиге фильтра    4 bytes
        uint32_t shiftSize;             ///< количество самплов зоны первичного суммирования              4 bytes
        int maxChannelsNumber;          ///< максимально возможное количество каналов    4 bytes
        char passwd[16];               ///< пароль доступа к настроечному режиму                          16 bytes
        CalibrateData calibrates[DEF_CALIBRATE_MAX][MAX_CHANNELS_NUMBER];   // 8 * 5 * 8 = 320 bytes
        Zone zones[MAX_SUMMS_NUMBER];  ///< определение зон суммирования             4 * 3  = 12 bytes
        int phisic_chn[MAX_CHANNELS_NUMBER]; //< номера физически опрашиваемых каналов АЦП или UDP                 4 * 8 = 32 bytes
        int logic_chn[MAX_CHANNELS_NUMBER]; //< номера каналов отображенных в программе                     4 * 8 = 32 bytes
        int address_chn[MAX_CHANNELS_NUMBER]; //< IP-address канала для UDP                     4 * 8 = 32 bytes
        int type_chn[MAX_CHANNELS_NUMBER]; //< Тип канала дпггых 
        float minR2;     ///< порог коэффициента детерминации для ошибок       (4*8 = 32 байта)
        int errLength;     ///< минимальная длина превышения kk для ошибки     (4*8 = 32 байта)
        int calibrateMax;                       ///< количество калибровочных уровней; 4 bytes
        int calibrateCurrent;                       ///< текущий калибровочный уровень (0 - calibrateMax); 4 bytes
        float K_Disp;                           ///< коэффициент для вычисления текущей дисперсии (близко, но < 1) 4 bytes
        float K_MO;                             ///< коэффициент для вычисления текущего мат. ожидания (близко, но < 1) 4 bytes
        float Num_Sigma;                        ///< множитель сигма для вычисления мин./макс. значения отклонения для пропуска итерации 4 bytes
        float Num_Sigma_F;                        ///< множитель сигма для срабатывания фильтра обработки ошибок нижнего уровня \ref SigmaClassicFilter
        float Len_Sigma_F;                        ///< длина фильтра обработки ошибок нижнего уровня \ref SigmaClassicFilter
        long avrSlow;                           ///< длина усреднения для дисперсии и МО
        int parabola_len;                       ///< заданая длина буферов оъекта парабола
        int parabola_min;                       ///< заданное минимальное значение длины для начала апроксимации
    };
};
typedef settings_data SettingsData;

#define SOFT_NOT_WORKING        0xFF00
#define SOFT_WAIT_CALIBRATE     2
#define SOFT_IS_MARKED          4

#define STATES_DATA_SIZE 512 ///< Длина структуды текущих данных состояния ПО и оборудования
/**\brief Структура текущих данных состояния ПО и оборудования
 */
union states_data{
    char data[STATES_DATA_SIZE];
    struct{
        uint16_t hardState;                     ///< текущее слововостояние работы оборудования   2 bytes
        uint16_t softState;                     ///< текущее словосостояние работы софта          2 bytes
        uint16_t typeReader;                    ///< тип источника данных                         2 bytes
        int lastGrPosition;                     ///< позиция последней записанной порции          2 bytes
        int currentGrBufferSize;                  ///< текущий размер буфера [0..MAX_GRAPHIC_OBJECTS - 1]   2 bytes
        uint8_t dumpStore;                      ///< состояние записи исходных данных ДМА         1 bytes
        uint8_t summStore;                      ///< состояние записи сумм данных                 1 bytes
        uint8_t isCalibrateMode;                ///< состояние разрешения калибровки              1 bytes
        uint8_t nonStopMode;                    ///< состояние без останова конвейера             1 bytes
        int waitHardReady;                      ///< ожидание готовности оборудования -1 -ready   2 bytes
        int waitCompliteStatistic;              ///< ожидание готовности статистики -1 -ready     2 bytes
        long averagesCount;                     ///< текущий счетчик цмклов принятия решения      4 bytes
        long lastAveragesCount;                 ///< предыдущее значение счетчика циклов принятия решения 4 bytes
        int calibrateCurrent;                   ///< текущий калибровочный уровень (0 - calibrateMax);
        int countEvents;                        ///< счетчик сработок;
        int countNonGabarites;                  ///< счетчик негабаритов;
        int countLowFilterChange;                  ///< счетчик сработок фильтра ошибок нижнего уровня \ref SigmaClassicFilter;
        int lastMaxLen[MAX_CHANNELS_NUMBER];        ///< длина превышения порога в последнем случае превышения порога
        float lastMaxSigma[MAX_CHANNELS_NUMBER];    ///< sigma? достигнутая в последнем случае превышения порога
        uint32_t lastHState;                    ///< маска последеннего состояния оборудования
        //        int errLen;
    };
};
typedef states_data StatesData;
/**\brief Структура параметров команд SET_CALIBRATE_DATA/GET_CALIBRATE_DATA
 */
typedef struct{
    int calibrateNum;   ///< Номер уровня чувствительности, -1 - текущий уровень
    int channelNum;     ///< Номер канала, -1 - все каналы
    float sigma;        ///< пороговое значение Sigma
    int length;         ///< пороговое значение длины превышения порога
} ShmCommandSetCalibrateData;

#define PARSER_HEADER_DATA_SIZE  (CALIBRATE_DATA_SIZE * MAX_CHANNELS_NUMBER + SETTINGS_DATA_SIZE + STATES_DATA_SIZE)
union parser_header{
    char data[PARSER_HEADER_DATA_SIZE];
    struct{
        CalibrateData calibrate[MAX_CHANNELS_NUMBER];           // 512 bytes
        SettingsData settings;                                  // 512 bytes
        StatesData states;                                      // 64 bytes
    };
};
typedef parser_header ParserHeader;

#define HARD_HEADER_DATA_SIZE

#define DEF_PLC_ID            2
#define DEF_MATRIX_ID         1
#define DEF_PLC_SPEED         19200L
#define DEF_MATRIX_SPEED      19200L
#define DEF_PLC_TTY                0
#define DEF_MATRIX_TTY             1

union hard_header{
    char data[HARD_HEADER_DATA_SIZE];
    struct{
        int protecterTTYNum;
        long protectedSpeed;
        int mbProtectedAddress;
        int mbProtectedDebug;       // 0 - false; 1 - debug; -1 - no present
        int mbProtectedRegsInpAddress;
        int mbProtectedRegsInpCount;
        int mbProtectedRegsOutAddress;
        int mbProtectedRegsOutCount;
        int displayTTYNum;
        long displaySpeed;
        int mbDisplayAddress;
        int mbDisplayDebug;         // 0 - false; 1 - debug; -1 - no present
        int mbDisplayRegsRWAddress;
        int mbDisplayRegsRWCount;
        int mbDisplayBitsRWAddress;
        int mbDisplayBitsRWCount;
    } settings;
};
typedef hard_header HardHeader;

enum  SHMCommandId {
// Команды Web Сервера    
    NO_COMMAND = 0,
    GET_GRAPHICS,
    GET_CALIBRATE_DATA, SET_CALIBRATE_DATA, STORE_CALIBRATE, READ_CALEBRATE,
    SET_CALIBRATE_NUM, GET_CALIBRATE_NUM,
    GET_STATES, SET_STATES, SET_TUNNING,
    GET_TIME, SET_TIME,
    GET_BIN_DUMP, SET_BIN_DUMP,
    GET_SUM_DUMP, SET_SUM_DUMP,
    GET_FILE,
    GET_ALL_STATES4,
    GET_ALL_STATES5,
    GET_ALL_STATES6,
    SAVE_AS_RECEPT,
    SET_ZONE,
    GET_ZONES,
// Команды modbus логгера    
    PROGRAMM_STATED,
    PROGRAMM_STOPED,
    SET_EVENT,
    PARSER_RECALIBRATE,
    SET_LED,
    PROTECTED_UP_DOWN,
    SET_AUTO_MODE,
    HARD_REQUERY,
    GET_PROTECTED_STATE,
    CLEAR_COUNTS,
    CONVERYOR_STOP,
    SET_NON_STOP_MODE,
    CLEAR_TEMP_LEVEL,

    STATE_PROTECTED_CONV_STOP,
    STATE_PROTECTED_CONV_START,
    STATE_PROTECTED_UP,
    STATE_PROTECTED_DOWN,
    STATE_PROTECTED_B_HALT_DOWN,
    STATE_PROTECTED_B_HALT_UP,
    STATE_RZO_FINDED,
    ERR_PROTECTED_OPTICAL,
    ERR_PROTECTED_MOVED,
    ERR_MODBUS_NET,
    ERR_PARSER,
    ERR_READER,
    ERR_DMA_READ_QUERY,
    START_RECALIBRATE,
    STOP_RECALIBRATE,
    ERR_LOGGER_QUERY,
    REBOOT_NEEDED
// другие переменные состояния и/или команды
//    ERR_READER_QUERY,
//    ERR_READER_AMPFILLER,

};

#define STATE_OK                0
#define STATE_OBJECT_ERROR      (0x80000000)

// Состояния ПМ-ошибок
#define HADR_FATAL_MOTION                   0x01    // фатальная ошибка привода
#define HARD_OPTICAL_ERROR                  0x02    // ошибка оптических датчиков
#define HARD_HALT_BUTTON_DOWN               0x04    // кнопка "Гриб" нажата
#define HARD_CONV_IS_STOP                   0x08    // конвейер стоит
#define HARD_COIL_IS_UP                     0x10    // каретка не в нижнем положении
#define HARD_NOT_AUTO_MODE                  0x20    // защитное устройство в ручном режиме управления - регламент, сработки не считать

#define ERR_MODBUS_READ_WRITE               0x40    // ошибка приема/передачи/инициализации модбас связи
#define ERR_READER_QUERY                    (STATE_OBJECT_ERROR + 0x100)   // ошибка очереди ридера
#define ERR_READER_AMPFILLER                0x200   // ошибка усилителя
#define ERR_READER_STORE_DATA               0x1000  // ошибки возникающие в процессе открытия/записи данных

// для определения реакции парсера на усреднение и генерации сработок
/****************************** В Ачински посмотреть как будет работать ********************************************/
#define DONT_AVERAGE_BY_HARD_MASK           (HARD_HALT_BUTTON_DOWN  | HARD_COIL_IS_UP)  // каретка не в нижнем положении и/или нажат гриб - средние значения не считать
#define DONT_EVENT_GENERATE_BY_HARD_MASK    (DONT_AVERAGE_BY_HARD_MASK | HARD_CONV_IS_STOP)  // каретка не в нижнем положении | 

/* Структура предачи комманд/состояний между отдельными компонентами программы или между процессами */

#define SHM_COMMANDS_DATA_SIZE  32
union shm_commands_data {
    char data[SHM_COMMANDS_DATA_SIZE];
    struct {
        SHMCommandId commandId;
        uint8_t      complite;
        uint16_t     parameter;
        char         addParams[27];
    };
};
typedef shm_commands_data ShmCommandsData;

/********************************/
/* Структура расшаренной памяти */
/********************************/

#define MAX_SHMEM_CONNANDS_NUMBER   10

#define SHM_DATA_SIZE (COMMON_HEADER_DATA_SIZE + SETTINGS_DATA_SIZE + STATES_DATA_SIZE + (SM_DATA_SIZE * MAX_GRAPHIC_OBJECTS) + (SHM_COMMANDS_DATA_SIZE * MAX_SHMEM_CONNANDS_NUMBER))
typedef struct{
        CommonHeader        cheader;
        SettingsData        settings;
        StatesData          states;
//        ParserHeader        pheader;
        SummsRatiosData     smData[MAX_GRAPHIC_OBJECTS];
        ShmCommandsData     commands[MAX_SHMEM_CONNANDS_NUMBER];
} ShmData;

#define AIO64_MAX_QUEUE       MAX_GRAPHIC_OBJECTS      // значение длины очереди записи данных

#define TO_STRING_INT_VALUE     0
#define TO_STRING_LONG_VALUE    1
#define TO_STRING_FLOAT_VALUE   2
#define TO_STRING_EXP_VALUE     3

/**
 @brief Класс статических методов для выполнения разнообразных вспомогательных функций
 * 
 *  Класс статических методов для выполнения разнообразных вспомогательных функций.
 *  пример использования:  
 */

class Helper {
    static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems){
       std::stringstream ss(s,std::ios_base::in);
       std::string item;
       while(std::getline(ss, item, delim)) {
           elems.push_back(item);
       }
       return elems;
    }
public:
    Helper();
    Helper(const Helper& orig);
    virtual ~Helper();
    
//    template <typename>
//    static std::string toString(void *T, int type){
//        char b[25]; memset(b,0,25);
//        switch(type){
//            case TO_STRING_INT_VALUE:
//                sprintf(b,"%d", *((int*)value)); break;
//            case TO_STRING_LONG_VALUE:
//                sprintf(b,"%ld", *((long*)value)); break;
//            case TO_STRING_FLOAT_VALUE:
//                sprintf(b,"%.12f", *((float*)value)); break;
//            case TO_STRING_EXP_VALUE:
//                sprintf(b,"%e", *((float*)value)); break;
//            default: b[0] = '\0';
//        }
//        std::string res = b;
//        return res;
//    };
    
    static std::vector<std::string> split(const std::string &s, char delim){
        std::vector<std::string> elems;
        return split(s, delim, elems);
    }
    inline static bool FileExists(const char* path){
        std::ifstream ff(path);
        if(ff.is_open()){
            ff.close();
            return true;
        } else {
            return false;
        }
    }
    static std::string &StrUpper (std::string &buffer){
        for(int i=0; i < buffer.size();i++){
            char c = buffer[i];
            char u = toupper(c);
            (buffer)[i] = u;
        }
        return buffer;
    }
    std::string &replace( std::string &src, const std::string &find_str, const std::string &replace_str ){
        std::string::size_type i = 0;
        while ( ( i = src.find( find_str, i ) ) != src.npos )
        {
            src.replace( i, find_str.length(), replace_str );
            i+=replace_str.length();
        }
        return src;
    } 
    int Trim(char *buffer){
        int n = strlen(buffer) - 1;
        while ( !isalnum(buffer[n]) && n >= 0 )
            buffer[n--] = '\0';

        return 0;
    }
    
    bool intCompare(int* first, int* second, int len){
    bool res = true;
        for(int i=0; i<len; i++){
            if(first[i] != second[i]){
                res = false;
                break;
            }
        }
        return res;
    }
    template <typename T>
    static std::string toString(T value){
        char b[50]; memset(b,0,50);
        if(typeid(T) == typeid(float)){
            sprintf(b, "%f", value);
        } else if(typeid(T) == typeid(double)){
            sprintf(b, "%e", value);
        } else if(typeid(T) == typeid(long)){
            sprintf(b, "%ld", value);
        } else {
            sprintf(b, "%d", value);
        }
        std::string res = b;
        return res;
    }
//    static std::string toString(int value){return toString(&value, TO_STRING_INT_VALUE);};
//    static std::string toString(long value){return toString(&value, TO_STRING_LONG_VALUE);};
//    static std::string toString(float value){return toString(&value, TO_STRING_FLOAT_VALUE);};
    
    static std::string getMyIP(){
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1){
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
            if (ifa->ifa_addr == NULL)
                continue;  
            s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            std::string name = ifa->ifa_name;
            if((name.compare(0, 3, "eth") == 0)&&(ifa->ifa_addr->sa_family==AF_INET)){
                name = host;
                return name;
            } else {
                continue;
            }
        }
        return "";
    };
};

#define STATE_SAVE_OPEN32       0x10

class CommonObject {
private:    
    
public:
    std::string fname;
    CommonObject(){
        cheader.channelsNumber   = DEF_CHANNELS_NUMBER;
        cheader.zonesNumber      = MAX_SUMMS_NUMBER;
        cheader.ratiosNumber     = MAX_RATIOS_NUMBER;
        state = STATE_OK;
        storedData = false;
        fd = -1;
        my_aio_count = 0;
        fname = "";
        for(int i=0; i<AIO64_MAX_QUEUE; i++)
            memset(&my_aio[i], 0, sizeof(struct aiocb));
    }
    CommonObject(const CommonObject& orig){
        this->state = orig.state;
        memcpy(&cheader, &orig.cheader, sizeof(CommonHeader));
    }
    CommonObject(CommonHeader *_cheader){
        if(_cheader){
            memcpy(&this->cheader, _cheader, sizeof(CommonHeader));
        } else {
            cheader.channelsNumber   = DEF_CHANNELS_NUMBER;
            cheader.zonesNumber      = MAX_SUMMS_NUMBER;
            cheader.ratiosNumber     = MAX_RATIOS_NUMBER;
        }
        state = STATE_OK;
        storedData = false;
        fd = -1;
        my_aio_count = 0;
        for(int i=0; i<AIO64_MAX_QUEUE; i++)
            memset(&my_aio[i], 0, sizeof(struct aiocb));
    }
    const CommonObject &operator =(const CommonObject &B){
        if(this == &B){
            return *this;
        }
        memcpy(&cheader, &B.cheader, sizeof(CommonHeader));
        this->state = B.state;
        return *this;
    }
    virtual ~CommonObject(){};
    
    inline CommonHeader *getCHeader(){ return &cheader;}
    inline int getChannelsNumber(){ return cheader.channelsNumber; }
    inline void setChannelsNumber(int _channels){ cheader.channelsNumber = _channels; }
    inline int getZonesNumber(){return cheader.zonesNumber;}
    inline void setZonesNumber(int _zones){cheader.zonesNumber = _zones; }
    inline int getRatiosNumber(){ return cheader.ratiosNumber;}
    inline void setRatiosNumber(int _ratios){ cheader.ratiosNumber = _ratios; }

    inline bool isError(){ return (state & STATE_OBJECT_ERROR); }
    inline uint32_t getState(){ return state;}
    inline uint32_t setState(uint32_t _state){ state = _state; return state;}
    inline bool isSetState(uint32_t _state){ return (state & _state);}
    inline uint32_t addState(uint32_t _state){ state |= _state; return state;}
    inline uint32_t clearState(uint32_t _state){ state = state & (~_state); return state; }
    virtual uint32_t setStoredData(bool on_off, int type = 0){ storedData = on_off; return STATE_OK; } // начать/остановить сохранение данных на носитель. 
                                                                             // параметры on_off -понятно
                                                                             // type - на усмотрение реализаций (для Parser 0 - dump DMA, иначе dump SUMM)
                                                                             // возврат 0 - успех; иначе тип ошибки неудача
    virtual bool isStoredData(){ return storedData; }
    

protected:
    int fd;         // дескриптор файла для записи
    uint32_t    state; // состояние объекта (STATE_OK=0, STATE_OBJECT_ERROR = 0x80000000)
    CommonHeader cheader;
    bool storedData;                  // индикатор состояния активности записи данных на носитель
    
    
    struct aiocb64 my_aio[AIO64_MAX_QUEUE]; // структура для возможности  в неблокирующем режиме записи данных на носитель
    int my_aio_count;                     // указатель на текущий экземпляр записи
    
    virtual void saveData(int fd, void *data, long size){
        if(fd > 0){
            if(isSetState(STATE_SAVE_OPEN32)){
                write(fd, data, size);
            } else {
                my_aio[my_aio_count].aio_fildes = fd;
                my_aio[my_aio_count].aio_sigevent.sigev_notify = SIGEV_SIGNAL;
                my_aio[my_aio_count].aio_sigevent.sigev_signo = SIGUSR1;
                if(size < 0){
                    my_aio[my_aio_count].aio_sigevent.sigev_value.sival_int = -1;
                } else {
                    my_aio[my_aio_count].aio_sigevent.sigev_value.sival_int = my_aio_count;
                }
                my_aio[my_aio_count].aio_buf    = data;
                my_aio[my_aio_count].aio_nbytes = abs(size);
                aio_write64(&my_aio[my_aio_count]);
                my_aio_count = (my_aio_count++) % AIO64_MAX_QUEUE;
            }
        }
    }
    virtual void cancelSaveData(int fd){
        if(!isSetState(STATE_SAVE_OPEN32)){
            aio_cancel64(fd, NULL);
        }
    }
                                                                                  // в потомках перепереопределить под конкретный класс
};

const struct sembuf semLock = {0,-1,0};
const struct sembuf semUnlock = {0,1,0};

#ifdef _SEM_SEMUN_UNDEFINED 
   union semun
   {
     int val;				//<= value for SETVAL
     struct semid_ds *buf;		//<= buffer for IPC_STAT & IPC_SET
     unsigned short int *array;		//<= array for GETALL & SETALL
     struct seminfo *__buf;		//<= buffer for IPC_INFO
   };
#endif


#endif	/* COMMONOBJECTS_H */

