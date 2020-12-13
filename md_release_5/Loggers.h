/* 
 * File:   ILogger.h
 * Author: longway
 *
 * Created on 8 июля 2016 г., 17:53
 */

#ifndef LOGGERS_H
#define	LOGGERS_H

#include <modbus/modbus.h>
#include <map>
#include <sys/time.h>

#include "../Common/CommonObjects.h"

#define PROTECTED_REGISTERS_ADDRESS_IN  512
#define PROTECTED_REGISTERS_COUNT_IN    8
#define PROTECTED_REGISTERS_NB_IN          16
    /*********************************************** Изменить после тестирования ****************************************/
#define PROTECTED_REGISTERS_ADDRESS_OUT  550
//#define PROTECTED_REGISTERS_ADDRESS_OUT  770
#define PROTECTED_REGISTERS_COUNT_OUT    6
#define PROTECTED_REGISTERS_NB_OUT       12

#define MAX_LOG_QUERY_SIZE  16
    
#define PR_IN_REGS_ADDRESS_AUTO_MODE        PROTECTED_REGISTERS_ADDRESS_IN
#define PR_IN_REGS_ADDRESS_SIREN            (PROTECTED_REGISTERS_ADDRESS_IN + 1)
#define PR_IN_REGS_ADDRESS_CONV_STOP        (PROTECTED_REGISTERS_ADDRESS_IN + 2)
#define PR_IN_REGS_ADDRESS_EVENTS           (PROTECTED_REGISTERS_ADDRESS_IN + 3)
#define PR_IN_REGS_ADDRESS_PM_UP            (PROTECTED_REGISTERS_ADDRESS_IN + 7)
#define PR_IN_REGS_ADDRESS_SOFT_NOT_READY   (PROTECTED_REGISTERS_ADDRESS_IN + 8)
#define PR_IN_REGS_ADDRESS_AMP_NOT_READY    (PROTECTED_REGISTERS_ADDRESS_IN + 9)

union PROTECTED_REGS_IN {
    uint16_t registers[PROTECTED_REGISTERS_COUNT_IN];
    struct {
        uint16_t    auto_enable;
        uint16_t    siren_on_off;
        uint16_t    conv_stop;
        union{
            uint16_t    ev_ch[4];
            struct{
                uint16_t    ev_ch_0;
                uint16_t    ev_ch_1;
                uint16_t    ev_ch_2;
                uint16_t    ev_ch_3;
            };
        };
        uint16_t    protected_up;
        uint16_t    soft_not_ready;
        uint16_t    ampfiler_not_ready;
    };
};

#define PROTECTED_OUT_REG_ADDRESS_CONV_ACTIVE     PROTECTED_REGISTERS_ADDRESS_OUT
#define PROTECTED_OUT_REG_ADDRESS_PM_DOWN         (PROTECTED_REGISTERS_ADDRESS_OUT + 1)
#define PROTECTED_OUT_REG_ADDRESS_FATAL_ERROR     (PROTECTED_REGISTERS_ADDRESS_OUT + 2)
#define PROTECTED_OUT_REG_ADDRESS_OPT_ERROR       (PROTECTED_REGISTERS_ADDRESS_OUT + 3)
#define PROTECTED_OUT_REG_ADDRESS_B_HALT          (PROTECTED_REGISTERS_ADDRESS_OUT + 4)
#define PROTECTED_OUT_REG_ADDRESS_READY           (PROTECTED_REGISTERS_ADDRESS_OUT + 5)

union PROTECTED_REGS_OUT {
    uint16_t registers[PROTECTED_REGISTERS_COUNT_OUT];
    struct {
        uint16_t    conv_active;
        uint16_t    protected_is_up;
        uint16_t    fatal_error;
        uint16_t    optical_error;
        uint16_t    bHalt;
        uint16_t    protected_is_ready;
    };
};

#define DISPLAY_REGS_RW_ADDRESS              50
#define DISPLAY_REGS_RW_COUNT                14
#define DISPLAY_REGS_RW_NB                   (DISPLAY_REGS_RW_COUNT * 2)
#define DISPLAY_REG_ADDRESS_DAY             DISPLAY_REGS_RW_ADDRESS
#define DISPLAY_REG_ADDRESS_MONTH           (DISPLAY_REGS_RW_ADDRESS + 1)
#define DISPLAY_REG_ADDRESS_YEAR            (DISPLAY_REGS_RW_ADDRESS + 2)
#define DISPLAY_REG_ADDRESS_HORSE           (DISPLAY_REGS_RW_ADDRESS + 3)
#define DISPLAY_REG_ADDRESS_MINUTES         (DISPLAY_REGS_RW_ADDRESS + 4)
#define DISPLAY_REG_ADDRESS_SECONDS         (DISPLAY_REGS_RW_ADDRESS + 5)
#define DISPLAY_REG_ADDRESS_EVENTS          (DISPLAY_REGS_RW_ADDRESS + 6)
#define DISPLAY_REG_ADDRESS_NON_GABARITES   (DISPLAY_REGS_RW_ADDRESS + 7)
#define DISPLAY_REG_ADDRESS_PO_STATE        (DISPLAY_REGS_RW_ADDRESS + 8)
#define DISPLAY_REG_ADDRESS_HARD_STATE      (DISPLAY_REGS_RW_ADDRESS + 9)
#define DISPLAY_REG_ADDRESS_CURRENT_LAVEL   (DISPLAY_REGS_RW_ADDRESS + 10)
#define DISPLAY_REG_ADDRESS_ERRORS          (DISPLAY_REGS_RW_ADDRESS + 11)
#define DISPLAY_REG_ADDRESS_CURRENT_SCREEN  (DISPLAY_REGS_RW_ADDRESS + 12)
#define DISPLAY_REG_ADDRESS_CALIB_NUM_TEMP      (DISPLAY_REGS_RW_ADDRESS + 13)


union DISPLAY_RW_REGS {
    uint16_t registers[DISPLAY_REGS_RW_COUNT];
    struct{
        uint16_t    day;                //50
        uint16_t    month;              //51
        uint16_t    year;               //52
        uint16_t    horse;              //53
        uint16_t    minutes;            //54
        uint16_t    seconds;            //55
        
        uint16_t    events;          // 56
        uint16_t    nonGabarits;     // 57
        uint16_t    poState;         // 58
        uint16_t    hardState;       // 59
        uint16_t    level_current;   // 60
        uint16_t    errors;          // 61
        uint16_t    screen_current;  // 62

        uint16_t    calibrate_num_temp;      // 63
    };
};

#define DISPLAY_BITS_RW_ADDRESS   19 
#define DISPLAY_BITS_RW_COUNT     11
#define DISPLAY_BITS_WRITE_NB     11

#define DISPLAY_BIT_TEMP_LAVEL_CH   (DISPLAY_BITS_RW_ADDRESS)
#define DISPLAY_BIT_CLEAR_COUNTS    (DISPLAY_BITS_RW_ADDRESS + 11)

union DISPLAY_RW_BITS{
    uint8_t bits[DISPLAY_BITS_RW_COUNT];
    struct{
        uint8_t alarms;         // 19 битовая маска сообщений
        uint8_t calibrate_change;   // 20   признак изменения значения уровня
        union{
            uint8_t buttons[5];
            struct{
                uint8_t button1; // 21
                uint8_t button2; // 22
                uint8_t button3; // 23
                uint8_t button4; // 24
                uint8_t button5; // 25
            };
        };
        uint8_t less;            //26 кнопка влево меньше
        uint8_t more;            //27 кнопка вправо больше
        uint8_t escape;          //28 кнопка Escape (сброс изменение уровня)
        uint8_t enter;           //29 кнопка Enter (сохранить изменение уровня)
        uint8_t clr;             //30 кнопка Clr (сброс счетчиков) 
    };
};

#define DISPLAY_ALARM_ADDRESS 80

struct RWData {
    modbus_t *ctx;
    int deviceId;
    int address;
    int count;
    pthread_mutex_t *mutex;
    
    uint16_t *source_dest_r;
    uint8_t *source_dest_b;
};

#define PR_STATE_CONV_NOT_ACTIVE        0x8
#define PR_STATE_IS_UP                  0x10
#define PR_STATE_FATAL_ERROR            0x1
#define PR_STATE_OPTICAL_ERROR          0x2
#define PR_STATE_B_HALT                 0x4
#define PR_STATE_MANUAL                 0x20

typedef enum mb_read_write{
    WRITE = false,
    READ
} MBReadWrite;

typedef enum mb_regs_bits{
    BITS = false,
    REGS
} MBRegsBits;

static std::map<SHMCommandId, std::string> logMessages = {
    {PROGRAMM_STATED, "Program started..."},
    {PROGRAMM_STOPED, "Programm stopped..."},
    {STATE_PROTECTED_CONV_STOP, "Conveyor stopped..."},
    {STATE_PROTECTED_CONV_START, "Conveyor started..."},
    {STATE_PROTECTED_UP, "Protected up now..."},
    {STATE_PROTECTED_DOWN, "Protected down now..."},
    {STATE_PROTECTED_B_HALT_DOWN, "Button halt pressed..."},
    {STATE_PROTECTED_B_HALT_UP, "Button halt released..."},
    {STATE_RZO_FINDED, "RZO finded..."},
    {ERR_PROTECTED_OPTICAL, "Error of optical sensors..."},
    {ERR_PROTECTED_MOVED, "Fatal error of protective gear motor..."},
    {ERR_MODBUS_NET, "Error of read/write modbus net..."},
    {SET_NON_STOP_MODE, "Program is non stop conveyor mode..."},
    {START_RECALIBRATE, "Program recalibrate now..."},
    {ERR_READER, "Warning... Error Reader query..."},
    {ERR_DMA_READ_QUERY, "Warning... READ count not eq DMA count..."},
    {ERR_PARSER, "Warning... Parser not working..."},
    {CLEAR_COUNTS, "Clear counts working..."},
    {SET_CALIBRATE_NUM, "Warning! Calibrate changed"},
    {PARSER_RECALIBRATE, "Warning! Parser recalibrate..."},
    {ERR_LOGGER_QUERY, "Error! Logger command query to long. more MAX_LOG_QUERY..."},
    {REBOOT_NEEDED, "ERROR! System reboot needed now..."}
};

class ILogger {
protected:
    HardHeader *_header;
    StatesData *_states;

    HardHeader *hheader;
    StatesData *pstates;
public:
    ShmCommandsData requeryCommand;

    ILogger();
    virtual ~ILogger();
    
    virtual uint32_t getHardState(bool _now=false) = 0;         // { return STATE_OK; };
    virtual int addLoggerState(ShmCommandsData command) = 0;
    int addLoggerState(SHMCommandId _commandId){
        ShmCommandsData cmd; 
        cmd.commandId = _commandId;
        return addLoggerState(cmd);
    }
    virtual void makeSyslogMsg(std::string _message) = 0;
    virtual void makeSyslogMsg(SHMCommandId command) {makeSyslogMsg(logMessages[command]);}
    virtual bool setNonStopMode(bool _nonStop){ pstates->nonStopMode = _nonStop; return pstates->nonStopMode; }
    virtual bool isNonStopMode(){return pstates->nonStopMode;}
    virtual void setShMemCommands(ShmData *_data) = 0;
    virtual int addDisplayLevel(int level) = 0;
//    inline void setPstates(StatesData* pstates) {
//        this->pstates = pstates;
//    }
//
//    inline StatesData* getStates() const {
//        return pstates;
//    }
};

extern     int shmid, semLockId;

class ModBusLogger : public ILogger{
    
    modbus_t *master_ctx_plc;
    modbus_t *master_ctx_display;

    union PROTECTED_REGS_OUT    protected_registers_out;
    union PROTECTED_REGS_IN     protected_registers_in;

    uint16_t countEvents;
    uint16_t countNonGabarites;
    uint16_t softState; // работа парсера: 1 - Регламент/работа, 2 - Ошибка, 4 - настройка (битовая маска)

    uint16_t hardState; // текущее состояние регистра ПМ: 0 - Ok;
    
    pthread_mutex_t plc_mb_mutex;       // мутексы для блокировки каналов RS-232 от перекресных запросов чтения/запмси
    pthread_mutex_t display_mb_mutex;
    pthread_mutex_t queryCommandMutex;
    pthread_cond_t queryCommandCond;
    
    pthread_t threadId;
    pthread_t displayId;
    
    itimerval requeryTimer;

    std::queue<ShmCommandsData> logQuery;
    RWData *tmpRWData;

    long lastAveragesCount;
    
    bool isPrepareReadWrite(MBReadWrite ReadWrite, int _device, int _addr, int _count, uint16_t *_sd, MBRegsBits regs_bits = REGS, RWData *data = NULL);
    bool isModBusRead(int device, MBRegsBits regs_bits = REGS, int _address = 0, int _count = 0, uint16_t *dest = NULL);
    bool isModBusWrite(int device, MBRegsBits regs_bits = REGS, int _address = 0, int _count = 0, uint16_t *dest = NULL);

    uint32_t hardRequery();

    uint32_t setEvents(ShmCommandsData _cmd);
    uint32_t setLight(uint8_t mask);
    uint32_t setLight(ShmCommandsData _cmd){
        return setLight(_cmd.parameter && 0x00FF);
    };
    
    virtual uint32_t getHardState(bool _now);

    
    bool isProtectedConvStop(uint16_t _state=0xFFFF);
    bool isProtectedUp(uint16_t _state=0xFFFF);
    bool isProtectedFatalMoved(uint16_t _state=0xFFFF);
    bool isProtectedOpticalError(uint16_t _state=0xFFFF);
    bool isProtectedBHalt(uint16_t _state=0xFFFF);
    bool isProtectedAutoMode(uint16_t _state=0xFFFF);

    bool isParserRunnting(bool complite = false){
        bool res = true;
        if(shmData->states.averagesCount != lastAveragesCount){
            res = true;
        } else {
            res = false;
        }
//        if(complite)
//            lastAveragesCount = shmData->states.averagesCount;
        return res;
    }
    bool isParserWaits(){
        if((shmData->states.softState & SOFT_WAIT_CALIBRATE) != 0){
           return true; 
        } else {
            return false;
        }
    }
    
    uint32_t setComplexReady(bool now=false){
            uint32_t ret;
            protected_registers_in.soft_not_ready = ((isParserRunnting()) && (!isParserWaits()) && (!isNonStopMode())) ? 0 : 1; 
            if(now)
                ret = (isModBusWrite(hheader->settings.mbProtectedAddress,REGS,PR_IN_REGS_ADDRESS_SOFT_NOT_READY,1,&protected_registers_in.soft_not_ready))? STATE_OK : ERR_MODBUS_READ_WRITE;
            else 
                return STATE_OK;
    }

    
    uint32_t setProtectedAutoMode(ShmCommandsData _cmd);
    uint32_t setConveyorStopInit();
    uint32_t setProtectedUpDown(ShmCommandsData _cmd);

    uint32_t displayRefresh();
    uint16_t getHardStateToDisplay(uint16_t _state=0xFFFF);  
    uint16_t getSoftStateToDisplay(uint16_t _state=0xFFFF);
    virtual void makeSyslogMsg(SHMCommandId command){
        if(logMessages.find(command) != logMessages.end()){
            this->makeSyslogMsg(logMessages[command]);
        }
    };
    void makeSyslogMsg(std::string _message){
            syslog(LOG_INFO, "%s", _message.c_str()); 
    };
    
    void clearRegisters();

public:
    union DISPLAY_RW_REGS    display_rw_registers;
    union DISPLAY_RW_BITS    display_rw_bits;
    bool isWriteLevelTempRegister;

    ShmData *shmData;
    
    ModBusLogger(HardHeader *_header = NULL, StatesData *_states = NULL);
    virtual ~ModBusLogger();
    
    int addDisplayLevel(int _lavel);
    int addLoggerState(ShmCommandsData command);
    int addLoggerState(SHMCommandId _commandId){ return ILogger::addLoggerState(_commandId); };
    static void *parseQueryThread(void *_self);
    static void *displayThread(void *_self);
    
    inline void setShMemCommands(ShmData *_data){ shmData = _data; }
    
    uint32_t setConveyorStop();
    
};


#endif	/* ILOGGER_H */

