/* 
 * File:   ILogger.cpp
 * Author: longway
 * 
 * Created on 8 июля 2016 г., 17:53
 */

#include "Loggers.h"

extern bool mbDebug;
extern bool noModBus;
extern bool noConsole;

uint16_t *pm_State;
int countUpTime = 0;
int countTimer = 0;
int countAlarmTime = 0;
extern bool withWatchdog;

extern ILogger *logger;
extern sembuf slock; 
extern sembuf sunlock; 
extern StatesData *states;

ILogger::ILogger(){
    _header = new HardHeader();
    _header->settings.displaySpeed = DEF_MATRIX_SPEED;
    _header->settings.displayTTYNum = DEF_MATRIX_TTY;
    _header->settings.mbDisplayAddress = DEF_MATRIX_ID;
    _header->settings.mbDisplayDebug = 0;
    
    _header->settings.protectedSpeed = DEF_PLC_SPEED;
    _header->settings.protecterTTYNum = DEF_PLC_ID;
    _header->settings.mbProtectedAddress = DEF_PLC_ID;
    _header->settings.mbProtectedDebug = 0;
    
    _states = new StatesData();
    memset(_states, 0, sizeof(StatesData));
    pstates = _states;
    hheader = _header;
}

ILogger::~ILogger(){
    if(!_header) delete _header;
    if(!_states) delete _states;
}

long time_last = 0;
long lastAveragesCount = -1;
int countErrAveragesCount = 0;

pthread_cond_t display_cond;
pthread_mutex_t display_mutex;


void MasterTimerExpiriece(int signo){
    if(signo == SIGALRM){       // Сигнал секундного таймера
        if(withWatchdog){       // Для работы программного watchdog
            if(states){
                if(lastAveragesCount != states->averagesCount){
                    countErrAveragesCount = 0;
                } else {
                    countErrAveragesCount++;
                }
                lastAveragesCount = states->averagesCount;
                if(countErrAveragesCount >= 300){
                    if(logger){
                        if(!noConsole){
                            fprintf(stdout,"Reboot now...\n");
                        }
                        logger->makeSyslogMsg(REBOOT_NEEDED);
                    }
//                        kill(ppid, SIGUSR1);
//                        system("ls ax");
                    sync();
                    reboot(RB_AUTOBOOT);
                }
            }
        }

        if(logger){
            
//            uint16_t rd = logger->getHardState(true);
//            system("date");
            countTimer++; if(countTimer>9) countTimer = 0;
            logger->addLoggerState(logger->requeryCommand);
#ifdef DEBUG
    if(!noConsole){
    struct timeval tp; struct timezone tz;
        gettimeofday(&tp, &tz);
        long time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
//        fprintf(stdout,"%d timeExpiriens %ld msec.(%ld sec. %ld usec)\n", countTimer, time - time_last, tp.tv_sec, tp.tv_usec);
        time_last = time;
    }
#endif        
            if(countTimer == 0){
            
                if(countUpTime >= 0){
                    if((*pm_State & PR_STATE_CONV_NOT_ACTIVE) == 0){
                       if((*pm_State & PR_STATE_IS_UP) != 0)
                          countUpTime--;
                    }
                }
                if(countUpTime == 0){
                    if((*pm_State & PR_STATE_CONV_NOT_ACTIVE) == 0){
                        if((*pm_State & PR_STATE_IS_UP) != 0){
                           ShmCommandsData cmd; cmd.commandId = PROTECTED_UP_DOWN; cmd.parameter = 1; 
                           logger->addLoggerState(cmd);
                           countUpTime--;
                        }
                    }
                }
//                if(countAlarmTime >= 0){
//                    countAlarmTime--;
//                }
            }
            if(countTimer == 5){
                pthread_mutex_lock(&display_mutex);
//                fprintf(stdout,"refresh display\n");
                pthread_cond_broadcast(&display_cond);
                pthread_mutex_unlock(&display_mutex);
            }
        }
    }
}

ModBusLogger::ModBusLogger(HardHeader *_header, StatesData *_states){
    if(_header) hheader = _header;
    if(_states) pstates = _states;
    
    shmData = NULL;
    
    std::string tty = "/dev/ttyS"+Helper::toString(hheader->settings.protecterTTYNum);
    master_ctx_plc = modbus_new_rtu(tty.c_str(), hheader->settings.protectedSpeed, 'N', 8, 1);
    
    if(master_ctx_plc == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context PLC\n");
    }
#ifdef DEBUG
    else {
        if(!noConsole){
            fprintf(stderr, "Connected PLC to %s speed %ld...\n", tty.c_str(), hheader->settings.protectedSpeed);
        }
    }
#endif
    int res = modbus_set_slave(master_ctx_plc, hheader->settings.mbProtectedAddress);
    if(res < 0){
        master_ctx_plc = NULL;
        fprintf(stderr, "ERROR: Unable modbus_set_slave context PLC\n");
    } 
#ifdef DEBUG
    else {
        if(!noConsole){
            fprintf(stderr, "modbus_set_slave context PLC to %d Ok.\n", hheader->settings.mbProtectedAddress);
        }
    }
#endif
    
    tty = "/dev/ttyS"+Helper::toString(hheader->settings.displayTTYNum);
    master_ctx_display = modbus_new_rtu(tty.c_str(), hheader->settings.displaySpeed, 'N', 8, 1);
    if(master_ctx_display == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context Display\n");
    }
#ifdef DEBUG    
    else {
        fprintf(stderr, "Connected Display to %s speed %ld...\n", tty.c_str(), hheader->settings.displaySpeed);
    }
#endif
    res = modbus_set_slave(master_ctx_display, hheader->settings.mbDisplayAddress);
    if(res < 0){
        master_ctx_display = NULL;
        fprintf(stderr, "ERROR: Unable modbus_set_slave context Display\n");
    }
#ifdef DEBUG
    else {
        if(!noConsole){
            fprintf(stderr, "modbus_set_slave context Display  to %d Ok.\n",hheader->settings.mbDisplayAddress);
        }
    }
#endif
    if(hheader->settings.mbProtectedDebug){
        modbus_set_debug(master_ctx_plc, true);
        printf("Start modbus_set_debug with plc\n");
    }
    
    modbus_set_error_recovery(master_ctx_plc,
                               (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK |
                               MODBUS_ERROR_RECOVERY_PROTOCOL));

    if(hheader->settings.mbDisplayDebug){
        modbus_set_debug(master_ctx_display, true);
        printf("Start modbus_set_debug with display\n");
    }
    
    modbus_set_error_recovery(master_ctx_display,
                               (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK |
                               MODBUS_ERROR_RECOVERY_PROTOCOL));

    clearRegisters();
    
    requeryTimer.it_interval.tv_sec = 0;
    requeryTimer.it_interval.tv_usec = 100000;
    requeryTimer.it_value.tv_sec = 0;
    requeryTimer.it_value.tv_usec = 100000;

#ifndef NOLOGGER
    signal(SIGALRM, &MasterTimerExpiriece);
    setitimer(ITIMER_REAL, &requeryTimer, NULL);
#endif    
    while(!logQuery.empty())
        logQuery.pop();
    
    pthread_mutexattr_t attr;
    
//    pthread_mutexattr_init(&attr);
//    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    
    pthread_mutex_init(&queryCommandMutex, NULL);
    pthread_cond_init(&queryCommandCond, NULL);
    pthread_mutex_init(&plc_mb_mutex, NULL);
    pthread_mutex_init(&display_mb_mutex, NULL);
    pthread_mutex_init(&display_mutex, NULL);
    pthread_cond_init(&display_cond, NULL);
    
    requeryCommand.commandId = HARD_REQUERY;
    pm_State = &protected_registers_out.protected_is_ready;
    
    
        pthread_create(&threadId, NULL, parseQueryThread, this);
    
        if(hheader->settings.mbDisplayDebug >= 0){
            pthread_create(&displayId, NULL, displayThread, this);
        }
        
    ShmCommandsData cmd; cmd.commandId = PROTECTED_UP_DOWN; cmd.parameter = 1;
    addLoggerState(cmd);
    
    setLight(0);
}

void ModBusLogger::clearRegisters(){
    memset(&protected_registers_in, 0, sizeof(protected_registers_in));
    memset(&protected_registers_out, 0, sizeof(protected_registers_out));
    
    memset(&display_rw_bits, 0, sizeof(display_rw_bits));
    memset(&display_rw_registers, 0, sizeof(display_rw_registers));
}

ModBusLogger::~ModBusLogger(){
}

int ModBusLogger::addDisplayLevel(int level)/*union DISPLAY_RW_REGS *regs, union DISPLAY_RW_BITS *bits, int level)*/{
    if(level < 1){ display_rw_registers.calibrate_num_temp--; if(display_rw_registers.calibrate_num_temp < 1) display_rw_registers.calibrate_num_temp = 1; }
    else if(level > 5){ display_rw_registers.calibrate_num_temp++; if(display_rw_registers.calibrate_num_temp > 5) display_rw_registers.calibrate_num_temp = 5; }
    else {
        display_rw_registers.calibrate_num_temp = level; 
    }
    
    if(display_rw_registers.calibrate_num_temp != display_rw_registers.level_current){
        display_rw_bits.calibrate_change = 1;
    } else {
        display_rw_bits.calibrate_change = 0;
    }
//    isModBusWrite(hheader->settings.mbDisplayAddress,REGS,DISPLAY_REG_ADDRESS_CALIB_NUM_TEMP,1,&display_rw_registers.calibrate_num_temp);
    isWriteLevelTempRegister = true;
    return display_rw_registers.calibrate_num_temp;
}

uint32_t ModBusLogger::hardRequery(){
    
        uint32_t ret = STATE_OK;

        
        uint16_t newHReady = 0;

// Прочитали регистр статуса РМ
        int res = isModBusRead(hheader->settings.mbProtectedAddress, REGS, PROTECTED_OUT_REG_ADDRESS_READY, 1, &protected_registers_out.protected_is_ready);

        if(res <= 0){
#ifdef DEBUG
            if(!noConsole){
//                fprintf(stdout, "error reading modbus PM from requery function...\n");
            }
#endif
           newHReady = hardState;
        } else {
            newHReady = protected_registers_out.protected_is_ready;
        }
        
        if(isProtectedConvStop(hardState) != isProtectedConvStop(newHReady)){
            if(isProtectedConvStop(newHReady)){
                addLoggerState(STATE_PROTECTED_CONV_STOP);
            } else {
                addLoggerState(STATE_PROTECTED_CONV_START);
                if(!isProtectedUp(newHReady) && !isProtectedAutoMode(newHReady)){
                    ShmCommandsData command; command.commandId = SET_AUTO_MODE; command.parameter = 1;
                    setProtectedAutoMode(command);
                }
            }
        }

        if(isProtectedUp(hardState) != isProtectedUp(newHReady)){
            if(isProtectedUp(newHReady)){
                addLoggerState(STATE_PROTECTED_UP);
            } else {
                addLoggerState(STATE_PROTECTED_DOWN);
            }
        }

        if(isProtectedOpticalError(hardState) != isProtectedOpticalError(newHReady)){
            if(isProtectedOpticalError(newHReady)){
                addLoggerState(ERR_PROTECTED_OPTICAL);
            }
        }
        if(isProtectedFatalMoved(hardState) != isProtectedFatalMoved(newHReady)){
            if(isProtectedFatalMoved(newHReady)){
                addLoggerState(ERR_PROTECTED_MOVED);
            }
        }
        if(isProtectedBHalt(hardState) != isProtectedBHalt(newHReady)){
            if(isProtectedBHalt())
                addLoggerState(STATE_PROTECTED_B_HALT_DOWN);
            else
                addLoggerState(STATE_PROTECTED_B_HALT_UP);
        }
/* На всякий случай переведем в автомат при работающем конвейере и опущенной катушке */        
        if((!isProtectedConvStop(newHReady) && !isProtectedUp(newHReady)) && !isProtectedAutoMode(newHReady)){
            ShmCommandsData command; command.commandId = SET_AUTO_MODE; command.parameter = 1;
            setProtectedAutoMode(command);
        }
        
/* Готовом и сохраняем оперативные данные с шаред мемори */        
        hardState = newHReady;
        softState = 0;
        if(shmData){
            if(lastAveragesCount == shmData->states.averagesCount){
                softState != SOFT_NOT_WORKING;
            } else {
                softState &= ~(SOFT_NOT_WORKING);
            }
        } else {
            softState &= ~(SOFT_NOT_WORKING);
        }

        if(shmData){
            semop(semLockId, &slock, 1);
                shmData->states.softState |= softState;
                shmData->states.hardState = hardState;
                lastAveragesCount = shmData->states.averagesCount;
            semop(semLockId, &sunlock, 1);
        }        
}

void *ModBusLogger::displayThread(void* _self){
    ModBusLogger *self = (ModBusLogger*)_self;
    union DISPLAY_RW_BITS d_bits_old;
    union DISPLAY_RW_REGS d_regs_old;

    struct timeval tp; struct timezone tz;
        
    
    while(1){
//        gettimeofday(&tp, &tz);
//        long time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
//        fprintf(stdout,"%d timeExpiriens %ld msec.(%ld sec. %ld usec) sleep.", countTimer, time - time_last, tp.tv_sec, tp.tv_usec);
//        time_last = time;

        pthread_mutex_lock(&display_mutex);
        pthread_cond_wait(&display_cond, &display_mutex);

//        gettimeofday(&tp, &tz);
//        time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
//        fprintf(stdout," %ld msec. wakeup.", time - time_last);
        if(!noModBus){
        memcpy(d_bits_old.bits, self->display_rw_bits.bits, sizeof(union DISPLAY_RW_BITS));
        memcpy(d_regs_old.registers, self->display_rw_registers.registers, sizeof(union DISPLAY_RW_REGS));
        
        if(self->isModBusRead(self->hheader->settings.mbDisplayAddress, REGS, DISPLAY_REG_ADDRESS_CALIB_NUM_TEMP, 1, &self->display_rw_registers.calibrate_num_temp)){

//            struct timeval tp; struct timezone tz;

//        gettimeofday(&tp, &tz);
//        time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
//        fprintf(stdout," 1: %ld", time - time_last);
//        time_last = time;
            if(self->display_rw_registers.calibrate_num_temp != d_regs_old.calibrate_num_temp){
                self->addDisplayLevel(self->display_rw_registers.calibrate_num_temp);
            }
        }
        if(self->isModBusRead(self->hheader->settings.mbDisplayAddress, BITS)){
//        gettimeofday(&tp, &tz);
//        long time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
//        fprintf(stdout," 2: %ld", time - time_last);
            if(self->display_rw_bits.less != d_bits_old.less){ // нажата кнопка меньше
                self->addDisplayLevel(0);
//                self->isWriteAllRegisters = true;
            }
            if(self->display_rw_bits.more != d_bits_old.more){ // нажата кнопка больше
                self->addDisplayLevel(6);
//                self->isWriteAllRegisters = true;
            }
//            if(self->display_rw_bits.button1 != d_bits_old.button1){ // нажата кнопка 1
//                self->addLevel(1);
//            }
//            if(self->display_rw_bits.button2 != d_bits_old.button2){ // нажата кнопка 2
//                self->addLevel(2);
//            }
//            if(self->display_rw_bits.button3 != d_bits_old.button3){ // нажата кнопка 3
//                self->addLevel(3);
//            }
//            if(self->display_rw_bits.button4 != d_bits_old.button4){ // нажата кнопка 4
//                self->addLevel(4);
//            }
//            if(self->display_rw_bits.button5 != d_bits_old.button5){ // нажата кнопка 5
//                self->addLevel(5);
//            }
            if(self->display_rw_bits.enter != d_bits_old.enter){ // нажата кнопка Enter
#ifdef DEBUG
                if(!noConsole){
//                    fprintf(stdout,"Pressed ENTER buttom\n");
                }
#endif
                if(self->display_rw_bits.calibrate_change != 0){
                    ShmCommandsData command;
                    command.commandId = SET_CALIBRATE_NUM; command.parameter = self->display_rw_registers.calibrate_num_temp-1;
                    self->addLoggerState(command);
//                    display_rw_bits.level_change = 0;
                }
            }
            if(self->display_rw_bits.escape != d_bits_old.escape){
                self->addDisplayLevel(self->display_rw_registers.level_current);
//                self->display_rw_registers.calibrate_num_temp = self->display_rw_registers.level_current;
//                self->display_rw_bits.calibrate_change = 0;
//                self->isWriteAllRegisters = true;
            }
            // кнопка очистки счетчиков
            if(self->display_rw_bits.clr != d_bits_old.clr){
                self->addLoggerState(CLEAR_COUNTS);
            }
        } else {
#ifdef DEBUG
    if(!noConsole)   {
       fprintf(stdout, "error reading modbus display from requery function...\n");
    }
#endif
        } 
        self->displayRefresh();
#ifdef DEBUG
    if(!noConsole){
        gettimeofday(&tp, &tz);
        long time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
//        fprintf(stdout," 3: %ld\n", time - time_last);
    }
#endif
        }
        pthread_mutex_unlock(&display_mutex);
        
    }
}

int ModBusLogger::addLoggerState(ShmCommandsData command){
//    char buf[160];
    int res = pthread_mutex_lock(&queryCommandMutex);

    if(logQuery.size() < MAX_LOG_QUERY_SIZE){
        logQuery.push(command);
    } else {
//        sprintf(buf,"%s %d",, logQuery.size());
        std::string str = logMessages[ERR_LOGGER_QUERY] + Helper::toString(logQuery.size());
        makeSyslogMsg(str);
        fprintf(stdout,"command in buffer: ");
        while(!logQuery.empty()){
#ifdef DEBUG
            if(!noConsole){
                fprintf(stdout, "%d ",logQuery.front().commandId);
            }
#endif
            logQuery.pop();
#ifdef DEBUG
            if(!noConsole){
                fprintf(stdout,"%s\n", str.c_str());
            }
        }
#endif
    }
        
    pthread_mutex_unlock(&queryCommandMutex);
    pthread_cond_broadcast(&queryCommandCond);
#ifdef DEBUG
        if(!noConsole){
            if(command.commandId != HARD_REQUERY){
//                std::string cn = "COMMAND_PROTECTED_UP_DOWD"
                    fprintf(stdout,"addLoggerState: add no periodical command %d\n", command.commandId);
            }
        }
#endif
    
}

void *ModBusLogger::parseQueryThread(void* _self){
    ModBusLogger *self = (ModBusLogger*)_self;
    
    self->countEvents = 0;
    self->countNonGabarites = 0;
    
    std::queue<ShmCommandsData> qCopy;

    while(1){
        int res = pthread_mutex_lock(&self->queryCommandMutex);
        
        if(self->logQuery.empty()){
            pthread_cond_wait(&self->queryCommandCond, &self->queryCommandMutex);
        }
        while(!self->logQuery.empty()){
            qCopy.push(self->logQuery.front());
                self->logQuery.pop();
        }
        pthread_mutex_unlock(&self->queryCommandMutex);
#ifdef DEBUG
//        if(!noConsole){
//            fprintf(stdout,".");
//        }
#endif
        
        while(!qCopy.empty()){
#ifdef DEBUG
            if(!noConsole){
                if(qCopy.size() > 1){ fprintf(stdout,"More 1 size %d\n", qCopy.size());}
            }
#endif
            ShmCommandsData command = qCopy.front(); qCopy.pop();
            switch(command.commandId){
// реакция на команды программы/ПМ/дисплей
                case SET_EVENT:{
                    struct eventMessageData *evData;
                    evData = (eventMessageData*)(&command.addParams[0]);
                    char buf[200] = "";
                    sprintf(buf, "MBL: Set events command with %x ch %d length %d sigma %f R2 %f CC %d\n", command.commandId, evData->ch, evData->length, evData->sigma, evData->R2, evData->countChange);
                    std::string msg = buf;
#ifdef DEBUG
                    if(!noConsole){
                        fprintf(stdout,"%s", buf);
                    }
#endif
                    if((self->hardState & (DONT_EVENT_GENERATE_BY_HARD_MASK)) == 0){
                        self->setEvents(command);

                        if(!self->isNonStopMode())
                            self->addLoggerState(CONVERYOR_STOP);
                        self->makeSyslogMsg(msg);
                    }
                }
                break;
                case PROTECTED_UP_DOWN:
#ifdef DEBUG
                    if(!noConsole){
                        fprintf(stdout, "Set protected command with %s parameter...\n", command.parameter == 0 ? "UP" : "DOWN");
                    }
#endif
                    self->setProtectedUpDown(command);
                    break;
                case CONVERYOR_STOP:
#ifdef DEBUG
                    if(!noConsole){
                        fprintf(stdout, "Set protected command conveyor stop...\n", command.parameter == 0 ? "TRY" : "STOP");
                    }
#endif
                    self->setConveyorStop();
                    break;
                case SET_LED:
#ifdef DEBUG
                    if(!noConsole){
                        fprintf(stdout, "Set protected command set leght with %x params...\n", command.parameter);
                    }
#endif
                    self->setLight(command.parameter && 0xFF);
                    break;
                case SET_AUTO_MODE:
#ifdef DEBUG
                    if(!noConsole){
                        fprintf(stdout, "Set protected auto mode command with %s parameter...\n", command.parameter == 0 ? "OFF" : "ON");
                    }
#endif
                    self->setProtectedAutoMode(command);
                    break;
                case HARD_REQUERY:
                    self->hardRequery();
                    break;
                    
// реакция на изменение состояний комплекса
                case PROGRAMM_STATED:
                case PROGRAMM_STOPED:
                case STATE_PROTECTED_CONV_STOP:
                case STATE_PROTECTED_B_HALT_DOWN:
                case STATE_PROTECTED_B_HALT_UP:
                case ERR_PROTECTED_OPTICAL:
                case ERR_PROTECTED_MOVED:
                case ERR_MODBUS_NET:
                case ERR_READER:
                case ERR_DMA_READ_QUERY:{
                    char buf[200];
                    sprintf(buf,"Warning... READ count not eq DMA count... on %d buffers", command.parameter);
                    std::string msg = buf;
                    self->makeSyslogMsg(msg);
                }
                break;
                case STATE_PROTECTED_CONV_START:
                    self->setLight(0);
                    break;
                case STATE_PROTECTED_DOWN:
#ifdef DEBUG
                    if(!noConsole){
                        fprintf(stdout, "state protected down out_reg=%x...\n", self->protected_registers_out.protected_is_ready);
                    }
#endif
                    command.commandId = SET_AUTO_MODE; command.parameter = 1;
                    self->setProtectedAutoMode(command);
                    countUpTime = -1;
                    break;
                case STATE_PROTECTED_UP:
#ifdef DEBUG
                    if(!noConsole){
                        fprintf(stdout, "state protected up command out_reg=%x...\n", self->protected_registers_out.protected_is_ready);
                    }
#endif
                    if(!self->isProtectedConvStop() && !self->isProtectedBHalt()){
#ifdef DEBUG
                        if(!noConsole){
                            fprintf(stdout, "set waiting up time...\n");
                        }
#endif
                        countUpTime = 7;
                        self->countNonGabarites = (self->countNonGabarites++) % 1000;
                        semop(semLockId, &slock, 1);
                            self->shmData->states.countNonGabarites = self->countNonGabarites; 
                        semop(semLockId, &sunlock, 1);
                    }
                    break;
                case SET_NON_STOP_MODE:
                {
                    self->pstates->nonStopMode = command.parameter;
#ifdef DEBUG                    
                    if(!noConsole){
                        fprintf(stdout, "set protected to NON_STOP_MODE to=%s...\n", logger->isNonStopMode() ? "TRUE" : "FALSE");
                    }
#endif
                }
                break;
                case SET_CALIBRATE_NUM:{
                    if(self->shmData){
                        bool find = false; 
                        for(int cn=0;cn<MAX_SHMEM_CONNANDS_NUMBER && !find;cn++){
                            semop(semLockId, &slock, 1);
                            if(self->shmData->commands[cn].commandId == NO_COMMAND){
                                memcpy(&self->shmData->commands[cn].data, command.data, sizeof(command));
                                countAlarmTime = 5;
                                find = true;
#ifdef DEBUG
                                if(!noConsole){
                                    fprintf(stdout,"Command SET_CALIBRATE_NUM placed in %d number...\n", cn);
                                }
#endif
                            }
                            semop(semLockId, &sunlock, 1);
#ifdef DEBUG
                            if(!find){
                                if(!noConsole){
                                    fprintf(stdout,"ERR! Command SET_CALIBRATE_NUM not placed...\n");
                                }
                            }
#endif
                        }
                        
                    }
                }
                break;
                case SET_CALIBRATE_DATA:{
                    if(self->shmData){
                        bool find = false; 
                        for(int cn=0;cn<MAX_SHMEM_CONNANDS_NUMBER && !find;cn++){
                            semop(semLockId, &slock, 1);
                            if(self->shmData->commands[cn].commandId == NO_COMMAND){
                                memcpy(&self->shmData->commands[cn].data, command.data, sizeof(command));
                                find = true;
#ifdef DEBUG
                                if(!noConsole){
                                    fprintf(stdout,"Command SET_CALIBRATE placed in %d number...\n", cn);
                                }
#endif
                            }
                            semop(semLockId, &sunlock, 1);
#ifdef DEBUG
                            if(!find){
                                if(!noConsole){
                                    fprintf(stdout,"ERR! Command SET_CALIBRATE not placed...\n");
                                }
                            }
#endif
                        }
                        
                    }
                }
                break;
                case CLEAR_COUNTS:{
                    self->countEvents = 0;
                    self->countNonGabarites = 0;
                    
                }
                break;
                    
            }

//            pthread_mutex_unlock(&self->queryCommandMutex);
        }
    }    
}

uint32_t ModBusLogger::setEvents(ShmCommandsData _cmd){
    uint8_t mask = 0x0;
    for(int i=0; i < DEF_CHANNELS_NUMBER; i++){
        if(_cmd.parameter & (1<<i) != 0){
            switch(i){
                case 0:
                case 1:
                    mask |= 1; break;
                case 2:
                    mask |= 2; break;
                case 3:
                    mask |= 4; break;
                case 4:
                case 5:
                    mask |= 8; break; 
            }
        }
    }
    countEvents = (countEvents++) % 1000;
    if(shmData){
        semop(semLockId, &slock, 1);
            shmData->states.countEvents = countEvents; 
        semop(semLockId, &sunlock, 1);
    }
    return setLight(0xFF);
//    return setLight(mask);
}

uint32_t ModBusLogger::displayRefresh(){
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime); timeinfo = localtime(&rawtime);
    
    display_rw_registers.day       = (uint16_t)(timeinfo->tm_mday-1);
    display_rw_registers.month     = (uint16_t)timeinfo->tm_mon;
    if(timeinfo->tm_year - 115 < 0) timeinfo->tm_year = 115; 
    display_rw_registers.year      = (uint16_t)(timeinfo->tm_year - 115);
    display_rw_registers.horse     = (uint16_t)timeinfo->tm_hour;
    display_rw_registers.minutes   = (uint16_t)timeinfo->tm_min;
    display_rw_registers.seconds   = (uint16_t)timeinfo->tm_sec;
    
    display_rw_registers.events = countEvents % 1000;
    display_rw_registers.nonGabarits = countNonGabarites % 1000;
    
    display_rw_registers.level_current = shmData->states.calibrateCurrent+1;
    
    if(display_rw_registers.calibrate_num_temp != display_rw_registers.level_current){
        display_rw_bits.calibrate_change = 1;
    } else {
        display_rw_bits.calibrate_change = 0;
    }
    display_rw_registers.hardState = getHardStateToDisplay();
    display_rw_registers.poState = getSoftStateToDisplay();
    if(countAlarmTime >= 0){
        uint16_t alm = countAlarmTime == 0 ? 0 : 1;
        isModBusWrite(hheader->settings.mbDisplayAddress,BITS,DISPLAY_ALARM_ADDRESS,1,&alm);
        countAlarmTime--;
    }

//    int count = 2;
    int count = 2;
    if(isWriteLevelTempRegister){

        bool res = isModBusWrite(hheader->settings.mbDisplayAddress, REGS, DISPLAY_REG_ADDRESS_CALIB_NUM_TEMP, 1, &display_rw_registers.calibrate_num_temp);
//        count = 0;
        isWriteLevelTempRegister = false;
    }
    
    bool res = isModBusWrite(hheader->settings.mbDisplayAddress, REGS, hheader->settings.mbDisplayRegsRWAddress, hheader->settings.mbDisplayRegsRWCount - count);
    bool res1 = isModBusWrite(hheader->settings.mbDisplayAddress, BITS, hheader->settings.mbDisplayBitsRWAddress, 2);
    
    setComplexReady(true);
    lastAveragesCount = shmData->states.averagesCount;
}

/******************************* Вспомогательные методы *************************/

uint32_t ModBusLogger::getHardState(bool _now){
    return protected_registers_out.protected_is_ready;
}

/* Команда зажечь лампочки */
uint32_t ModBusLogger::setLight(uint8_t mask){
    for(int i=0; i < 4; i++){
        if(mask & (1 << i)){
            protected_registers_in.ev_ch[i] = 1;
        } else {
            protected_registers_in.ev_ch[i] = 0;
        }
    }
    bool res = isModBusWrite(hheader->settings.mbProtectedAddress, REGS, PR_IN_REGS_ADDRESS_EVENTS, 4, protected_registers_in.ev_ch);
//    res &= isModBusWrite(hheader->settings.mdDisplayAddress, BITS, DEF_DISPLAY_BITS_WRITE_ADDRESS, 4, protected_registers_in.ev_ch);
    
    if(!res){
#ifdef DEBUG
        if(!noConsole){
            fprintf(stdout, "ERROR ligth command with %x parameter...\n", mask);
        }
#endif
        addLoggerState(ERR_MODBUS_NET);
    } else {
#ifdef DEBUG
        if(!noConsole){
            fprintf(stdout, "Set light command with %x parameter...\n", mask);
        }
#endif
        return STATE_OK;
    }
}

uint16_t ModBusLogger::getSoftStateToDisplay(uint16_t _state){
    uint16_t state = 1;
    
//    if(!isParserRunnting()){
//        state = 3;
//    } else 
        if(isProtectedFatalMoved() || isProtectedOpticalError()){
        state = 5;
    } else if(isParserWaits()){
        state = 4;
    } else if(isProtectedBHalt() || isProtectedUp()){
        state = 2;
    }
    return state;
}

uint16_t ModBusLogger::getHardStateToDisplay(uint16_t _state) {
    uint16_t state;
    uint16_t ret = STATE_OK; uint16_t ret1 = STATE_OK;
    
    if(_state == 0xFFFF) state = protected_registers_out.protected_is_ready;
    else state = _state;

    if(isProtectedConvStop()){
       ret = 5; 
    }
    if(isProtectedUp()){
        ret = 4;
    }
    if(isProtectedFatalMoved()){
        ret = 1;
    }
    if(isProtectedOpticalError()){
        ret = 2;
    }
    if(isProtectedBHalt()){
        ret = 3;
    }
//    if(ret == STATE_OK || ret == 5){
//        if(!isParserWaits()){
//            ret1 = 4;
//        }
//        if(!isParserRunnting()){
//            ret1 = 3;
//        }
//    } else {
//        ret1 = 3;
//    }
//    if(now){
//        isModBusWrite(hheader->settings.mbDisplayAddress,REGS,DISPLAY_REG_ADDRESS_PO_STATE,1,&ret1);
//        isModBusWrite(hheader->settings.mbDisplayAddress,REGS,DISPLAY_REG_ADDRESS_HARD_STATE, 1, &ret);
//    }
    return ret;
}


/* Проверка стоит ли конвейер */
bool ModBusLogger::isProtectedConvStop(uint16_t _state) {
    uint16_t st = _state == 0xFFFF ? protected_registers_out.protected_is_ready : _state;
    return ((st & HARD_CONV_IS_STOP) == HARD_CONV_IS_STOP);
}
/* Проверка поднята ли катушка */
bool ModBusLogger::isProtectedUp(uint16_t _state) {
    uint16_t st = _state == 0xFFFF ? protected_registers_out.protected_is_ready : _state;
    return ((st & HARD_COIL_IS_UP) == HARD_COIL_IS_UP);
}
/* Проверка ошибки датчиков положения каретки и/или защиты двигателя*/
bool ModBusLogger::isProtectedFatalMoved(uint16_t _state) {
    uint16_t st = _state == 0xFFFF ? protected_registers_out.protected_is_ready : _state;
    return ((st & HADR_FATAL_MOTION) == HADR_FATAL_MOTION);
}
/* Проверка ошибки оптических датчиков */
bool ModBusLogger::isProtectedOpticalError(uint16_t _state) {
    uint16_t st = _state == 0xFFFF ? protected_registers_out.protected_is_ready : _state;
    return ((st & HARD_OPTICAL_ERROR) == HARD_OPTICAL_ERROR);
}
/* Проверка нажат ли ГРИБ */
bool ModBusLogger::isProtectedBHalt(uint16_t _state) {
    uint16_t st = _state == 0xFFFF ? protected_registers_out.protected_is_ready : _state;
    return ((st & HARD_HALT_BUTTON_DOWN) == HARD_HALT_BUTTON_DOWN);
}
/* Проверка автоматического режима РМ */
bool ModBusLogger::isProtectedAutoMode(uint16_t _state) {
    uint16_t st = _state == 0xFFFF ? protected_registers_out.protected_is_ready : _state;
    return ((st & HARD_NOT_AUTO_MODE) != HARD_NOT_AUTO_MODE);
}


/* Команда перевода РМ в режимы 1 - авто; 0 - ручкой */
uint32_t ModBusLogger::setProtectedAutoMode(ShmCommandsData _cmd){
#ifdef DEBUG  
    if(!noConsole){
        fprintf(stdout,"command setAutoMode to %s\n", _cmd.parameter == 0 ? "MANUAL": "AUTO");
    }
#endif
    return (isModBusWrite(hheader->settings.mbProtectedAddress,REGS,PR_IN_REGS_ADDRESS_AUTO_MODE,1,&_cmd.parameter) ? STATE_OK : ERR_MODBUS_READ_WRITE);
}
/* Команда подъема/опускания каретки*/
uint32_t ModBusLogger::setProtectedUpDown(ShmCommandsData _cmd){
    ShmCommandsData cmd; cmd.commandId = SET_AUTO_MODE; cmd.parameter = 0;
    uint32_t ret = setProtectedAutoMode(cmd);
    ret |= (isModBusWrite(hheader->settings.mbProtectedAddress, REGS, PR_IN_REGS_ADDRESS_PM_UP, 1, &_cmd.parameter)) ? STATE_OK : ERR_MODBUS_READ_WRITE;
    return ret;
}

uint32_t ModBusLogger::setConveyorStop(){
    int count = 3; int i = 0; uint32_t res = STATE_OK;
#ifdef DEBUG
    if(!noConsole){
        fprintf(stdout,"Set_conveyor_init count=%d ", i);
    }
#endif
    while(i < count && (setConveyorStopInit() != STATE_OK)){
#ifdef DEBUG
        if(!noConsole){
            fprintf(stdout,"count=%d\n", i);
        }
#endif
        i++;
    }
    uint16_t one = 1; i = 0;
#ifdef DEBUG
    if(!noConsole){
        fprintf(stdout,"\nSet_conveyor_stop count=%d", i);
    }
#endif
    while(i < count && !(res = isModBusWrite(hheader->settings.mbProtectedAddress, REGS, PR_IN_REGS_ADDRESS_CONV_STOP, 1, &one))){
#ifdef DEBUG
        if(!noConsole){
            fprintf(stdout,"count=%d\n", i);
        }
#endif
        i++;
    }
#ifdef DEBUG
    if(!noConsole){
        fprintf(stdout,"\n");
    }
#endif
    return (res) ? STATE_OK : ERR_MODBUS_READ_WRITE;
}

uint32_t ModBusLogger::setConveyorStopInit(){
    uint16_t nol = 0;
    return (isModBusWrite(hheader->settings.mbProtectedAddress, REGS, PR_IN_REGS_ADDRESS_CONV_STOP, 1, &nol)) ? STATE_OK : ERR_MODBUS_READ_WRITE;
}

bool ModBusLogger::isModBusRead(int device, MBRegsBits regs_bits, int _address, int _count, uint16_t *_dest){
    int res = 0;
    RWData data;
        
    if(!noModBus){
        if(isPrepareReadWrite(READ, device, _address, _count, _dest, regs_bits, &data)){
                pthread_mutex_lock(data.mutex);
                if(regs_bits == REGS)
                    res = modbus_read_registers(data.ctx, data.address, data.count, data.source_dest_r);
                else
                    res = modbus_read_bits(data.ctx, data.address, data.count, data.source_dest_b);
                pthread_mutex_unlock(data.mutex);
            if(res != data.count){
                makeSyslogMsg(ERR_MODBUS_NET);
                return false;
            } else {
                return true;
            }
        } else {
            makeSyslogMsg(ERR_MODBUS_NET);
            return false;
        }
        return res > 0;
    }        
    return true;
}


bool ModBusLogger::isModBusWrite(int device, MBRegsBits regs_bits, int _address, int _count, uint16_t* _dest) {
    int res = 0;
    RWData data;
        
    if(isPrepareReadWrite(WRITE, device, _address, _count, _dest, regs_bits, &data)){
        if(!noModBus){
            pthread_mutex_lock(data.mutex);
            if(regs_bits == REGS)
                res = modbus_write_registers(data.ctx, data.address, data.count, data.source_dest_r);
            else
                res = modbus_write_bits(data.ctx, data.address, data.count, data.source_dest_b);
            pthread_mutex_unlock(data.mutex);
        } else {
            res = data.count;
        }
        if(res != data.count){
            makeSyslogMsg(ERR_MODBUS_NET);
            return false;
        } else {
            return true;
        }
    } else {
        makeSyslogMsg(ERR_MODBUS_NET);
        return false;
    }
    return res > 0;
}

bool ModBusLogger::isPrepareReadWrite(MBReadWrite ReadWrite, int _device, int _addr, int _count, uint16_t* _sd, MBRegsBits regs_bits, RWData *data) {
   data = data == NULL ? tmpRWData : data;
   if(_device == hheader->settings.mbDisplayAddress){
       if(hheader->settings.mbDisplayDebug >= 0){
            data->ctx = master_ctx_display;
            data->mutex = &display_mb_mutex;
       } else {
           return false;
       }
   } else {
       if(hheader->settings.mbProtectedDebug >= 0){
            data->ctx = master_ctx_plc;
            data->mutex = &plc_mb_mutex;
       } else {
           return false;
       }
   }
   if(ReadWrite == READ){ //READ
        if(regs_bits == REGS){
             if(!_sd){ // Регистры READ выходной адрес
                 data->source_dest_r = _device == hheader->settings.mbDisplayAddress ? display_rw_registers.registers : protected_registers_out.registers;}
             else { data->source_dest_r = _sd; }
             // входной адрес
             if(_addr <= 0){ data->address = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayRegsRWAddress : hheader->settings.mbProtectedRegsOutAddress; }
             else { data->address = _addr; }
             // количество данных
             if(_count <= 0){ data->count = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayRegsRWCount : hheader->settings.mbProtectedRegsOutCount; }
             else { data->count = _count;
             }
        } else { // Биты READ выходной адрес
             if(!_sd){ data->source_dest_b = _device == hheader->settings.mbDisplayAddress ? display_rw_bits.bits : NULL; }
             else {  data->source_dest_b = (uint8_t*)_sd; }
             // входной адрес
             if(_addr <= 0){ data->address = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayBitsRWAddress : 0; }  
             else { data->address = _addr; }
             // количество данных
             if(_count <= 0){ data->count = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayBitsRWCount : 0; } 
             else { data->count = _count; }
        }
   } else { // WRITE
        if(regs_bits == REGS){ // Регистры WRITE - выходной адрес
             if(!_sd){ data->source_dest_r = _device == hheader->settings.mbDisplayAddress ? display_rw_registers.registers : protected_registers_in.registers;} 
             else { data->source_dest_r = _sd; }
             // входной адрес
             if(_addr <= 0){ data->address = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayRegsRWAddress : hheader->settings.mbProtectedRegsInpAddress; } 
             else { data->address = _addr; }
             // количество данных
             if(_count <= 0){ data->count = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayRegsRWCount : hheader->settings.mbProtectedRegsInpCount; } 
             else { data->count = _count; }
        } else { // Быты WRITE выходной адрес
             if(!_sd){ data->source_dest_b = _device == hheader->settings.mbDisplayAddress ? display_rw_bits.bits : NULL; }
             else { data->source_dest_b = (uint8_t*)_sd; }
             // входной адрес
             if(_addr <= 0){ data->address = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayBitsRWAddress : 0; }
             else { data->address = _addr; }
             // количество данных
             if(_count <= 0){ data->count = _device == hheader->settings.mbDisplayAddress ? hheader->settings.mbDisplayRegsRWCount : 0; }
             else { data->count = _count; }
        }
   }   
   return true;
}
