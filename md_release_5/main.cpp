/* 
 * File:   main.cpp
 * Author: longway
 *
 * Created on 1 июля 2016 г., 14:20
 */

#include <cstdlib>
#include <termios.h>

#include "../Common/CommonObjects.h"
#include "Recepts.h"
#include "Summary.h"
#include "BinFileReader.h"
#include "Loggers.h"
#include "Parsers.h"
#include "DMAReader.h"
#include "SummFileReader.h"
#include "UdpReader.h"

using namespace std;

/*
 * 
 */

#define IO_SIGNAL SIGUSR1

bool noConsole = false;
bool mbDebug = false;
bool noModBus = false;
bool wrEvents = false;
bool withWatchdog = false;
StatesData *states = NULL;

ILogger *logger = NULL;
AParser *parser = NULL;
IReader *reader = NULL;

static void aioSigWriteComplte(int sig, siginfo_t *si, void *ucontext){
    if(sig == IO_SIGNAL){
//        if(si->si_int == MAX_GRAPHIC_OBJECTS - 1){
//            if(parser->isStoredData()){
////                parser->setStoredData(false);
////                parser->getReader()->setStoredData(false);
//            }
//            
//        }
        if(si->si_int < 0){
           if(reader){
               reader->setStoredData(false);
           } 
        }
    }
}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
//    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
//    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

//    alarm(SIGALRM);
    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
//    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
      return ch;
    }

    return 0;
}
//#ifdef DEBUG
void printHelp(){
    printf("******************************\n");
    printf("\nУправление:\n");
    printf("    q - quit program\n");
    printf("    0-9, f, c - Led Events\n");
    printf("    p - Pause/suspend reader running\n");
    printf("    d - Start/Stop write dump file\n");
    printf("    s - Start/Stop write sdump file\n");
    printf("    t - up coil function\n");
    printf("    b - down coil function\n");
    printf("    n - switch to NON STOP Mode\n");
    printf("    пробел - маркер\n");
//    printf("    v - reset timer");
    printf("and you choise:");
}
//#endif


sembuf slock = {0, -1, SEM_UNDO}; sembuf sunlock = {0,1,0}; 
int shmid, semLockId;

extern long ref_count;


int main(int argc, char** argv) {

    float y[50];
    
    float a = 1, b = 2, c = 3, d = 4;
    int X[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    float Y[] = {-2.5, -6.7, -7.9, -8.5, -6.7, -6.3, -3.3, 0.3, 4, 9.2};
//    float a = -1, b = 2, c = 3;

    AParabola *parabola = new AParabola(0, 0, NULL);
    AParabola *parabola2 = new AParabola(0, 0, NULL);
    ev_parabols *result;
    ev_parabols *result2;
    
    for(int x=0; x < 10; x++){
        y[x] = a*(x*x) + b * x + c;
        
        parabola->addNext(Y[x], X[x], x);
        parabola2->addNext(y[x], x, x);
        result = parabola->getParams();
        result2 = parabola2->getParams();
    }


//    Parabola3 *parabola3 = new Parabola3(25);
//    ev_parabols *result3;
//    
//    for(int x=0; x < 50; x++){
//        y[x] = a * (x*x*x) + b * (x*x) + c * x + d;
//        
//        parabola3->addNext(x, y[x]);
//        result3 = parabola3->complite(1);
//    }


    SigmaClassic _sigma(1000);
    
#define RAT 1.23525846
    
    for(int i=0; i<1000; i++){
        _sigma.addNext(RAT);
        if(_sigma.getSigma() > 0){
            int y = 0;
        }
    }
    
    struct sigaction sig_action;
    memset(&sig_action, 0, sizeof(sig_action));
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_sigaction = aioSigWriteComplte;
    sig_action.sa_flags = SA_RESTART | SA_SIGINFO;    

    if (sigaction(IO_SIGNAL, &sig_action, NULL) == -1)
           fprintf(stdout, "sigaction not complite *****************\n");
        else
           fprintf(stdout, "sigaction complite +++\n");

    int count = 1;
    int typeR = TYPE_READER_DMA;
    std::string fname = "";
    while(count < argc){
        if(!strncmp(argv[count],"-nocon",5)){
            noConsole = true;
            count++;
            continue;        
        }
        if(!strncmp(argv[count], "-mbdebug",5)){
            mbDebug = true;
            count++;
            continue;
        }
        if(!strncmp(argv[count], "-nomodbus",6)){
            noModBus = true;
            count++;
            continue;
        }
        if(!strncmp(argv[count], "+whatch",6)){
            withWatchdog = true;
            count++;
            continue;
        }
        if(!strncmp(argv[count], "-whatch",6)){
            withWatchdog = false;
            count++;
            continue;
        }
        if(!strncmp(argv[count], "-f", 2)){
            typeR = TYPE_READER_BIN;
            withWatchdog = false;
            if(argc > count+1){
                if(argv[count+1][0] != '-'){ // не параметер - имя файла
                    if(argv[count+1][0] != '/'){ // нет пути
                        fname = "/home/longway/Common/";
                    }
                    fname += argv[count+1];
                    count++;
                }
            }
            if(fname == ""){
                fname = "/home/longway/Common/dump.bin";
            }
        }
        if(!strncmp(argv[count], "-s", 2)){
            typeR = TYPE_READER_SUM;
            withWatchdog = false;
            if(argc > count+1){
                if(argv[count+1][0] != '-'){ // не параметер - имя файла
                    if(argv[count+1][0] != '/'){ // нет пути
                        fname = "/home/longway/Common/";
                    }
                    fname += argv[count+1];
                    count++;
                }
            }
            if(fname == ""){
                fname = "/home/longway/Common/sdump.bin";
            }
        }
        if(!strncmp(argv[count], "-u", 2)){
            typeR = TYPE_READER_UDP;
        }
        if(!strncmp(argv[count], "-wevents", 7)){
            wrEvents = true;
        }
//        if(!strcmp(argv[count],"-sf")){
//            if(argc > count+1){
//                if(argv[count+1][0] != '-'){
//                    strcpy(sfileName, argv[count+1]);
//                    count++;
//                }
//            }
//            reader = new SummFileReader(sfileName);
//            count++;
//            continue;
//        }
//        if(!strcmp(argv[count],"-f")){
//            if(argc > count+1){
//                if(argv[count+1][0] != '-'){
//                    strcpy(bfileName, argv[count+1]);
//                    count++;
//                }
//            }
//            reader = new BinFileReader(bfileName);
//            count++;
//            continue;
//        }
        count++;
    }
    ShmCommandsData cmd;  
    ShmCommandSetCalibrateData *data = (ShmCommandSetCalibrateData*)&cmd.data;
    
    openlog("md_release", LOG_NDELAY | LOG_PID, LOG_USER);
    
    Recepts *sets = new Recepts();
    
    
//    IReader *reader;
    
    if(typeR == TYPE_READER_BIN){
        reader = new BinFileReader(fname.c_str(), sets->getCHeader());
//        reader = new BinFileReader("/home/longway/Common/dump.bin", sets->getCHeader());
    } else if(typeR == TYPE_READER_SUM){
        reader = new SummFileReader(fname.c_str(), sets->getCHeader());
//        reader = new SummFileReader("/home/longway/Common/sdump.bin", sets->getCHeader());
    } else if(typeR == TYPE_READER_UDP){
        reader = new UdpReader(sets->getSettings());
//        reader = new UdpReader(sets->getSettings());
    } else {
        reader = new DMAReader(sets->getSettings());
//        reader->setStoredData(true);
        if(wrEvents)
            reader->addState(READER_IS_STORED_COUNT_DATA);
//        withWatchdog = true;
    }

    parser = new AParser(reader, sets);

    logger = new ModBusLogger(sets->getHHeader(), parser->states);
    logger->setShMemCommands(parser->shmData);

    sleep(1);
   
    parser->start();
    
    SettingsData *settings = parser->settings;
    states = parser->states;
    
    while(1){
        if(noConsole){
            sleep(10);
//            pthread_join(parser->thread_id, NULL);
            continue;
        } else {
            printHelp();
            int aa = kbhit();
            fprintf(stdout, "\nchoise **************%c(%d)*******************\n", aa, aa);
            if(aa == 'q'){
                break;
            } else {
                switch(aa){
                    case 'd':{
                        reader->setStoredData(true, 666);
//                        if(reader->isStoredData()){
//                            reader->setStoredData(false);
//                        } else {
//                            reader->setStoredData(true, 0);
//                        }
                    }
                    break;
                    case 's':{
//                        if(parser->isStoredData()){
//                            parser->setStoredData(false);
//                        } else {
//                            parser->setStoredData(true, 0);
//                        }
                        parser->storeAllGaraphicsData(666);
                    }
                    break;
                    case 'p':{
                        if(reader->isSetState(READER_IS_STOPPED)){
                            parser->pause(false); // снять с паузы
                        } else {
                            parser->pause(true); // поставить на паузу
                        }
                    }
                    case '0': {
                        if(logger){
                            logger->addLoggerState(STORE_CALIBRATE);
                        }
                    }
                        break;
                    case '1': case '2': case '3':/* case '4': case '5':*/{
                        if(logger){
                            cmd.commandId = SET_CALIBRATE_NUM, cmd.parameter = aa - '1';
                            logger->addLoggerState(cmd);
                        }
                    }
                        break;
                    case '4': case '5': case '6': case '7':  
                        cmd.commandId = SET_LED; cmd.parameter = aa-'0';
    //                    fprintf(stdout, "send events command whith %x\n", aa - '0');
                        logger->addLoggerState(cmd);
                    break;
                    case '8':{
                        data->calibrateNum = states->calibrateCurrent;
                        data->channelNum = -1;
                        data->length = -1;
                        if(settings->calibrates[states->calibrateCurrent][0].pSigma > settings->Num_Sigma){
                            data->sigma = settings->calibrates[states->calibrateCurrent][0].pSigma - 1;
                        }
                        cmd.commandId = SET_CALIBRATE_DATA;
                        if(logger){
                            logger->addLoggerState(cmd);
                        }
                    }
                    break;
                    case '9':{
                        data->calibrateNum = states->calibrateCurrent;
                        data->channelNum = -1;
                        data->length = -1;
                        data->sigma = settings->calibrates[states->calibrateCurrent][0].pSigma + 1;
                        cmd.commandId = SET_CALIBRATE_DATA;
                        if(logger){
                            logger->addLoggerState(cmd);
                        }
                    }
                    break;
                    case 'f':
                        cmd.commandId = SET_LED; cmd.parameter = 0xFF;
                        logger->addLoggerState(cmd);
                        break;
                    case 'c':
                        cmd.commandId = SET_LED; cmd.parameter = 0x0;
                        logger->addLoggerState(cmd);
                        break;
                    case 't':
                        cmd.commandId = PROTECTED_UP_DOWN; cmd.parameter = 1;
                        logger->addLoggerState(cmd);
                        break;
                    case 'b':
                        cmd.commandId = PROTECTED_UP_DOWN; cmd.parameter = 0;
                        logger->addLoggerState(cmd);
                        break;
                    case 'n':{
                        if(logger){
                            cmd.commandId = SET_NON_STOP_MODE;
                            cmd.parameter = logger->isNonStopMode() ? false : true;
      fprintf(stdout, "!!!!!!!!!!!!set protected to NON_STOP_MODE to=%s...\n", logger->isNonStopMode() ? "TRUE" : "FALSE");
                            logger->addLoggerState(cmd);
                        }
                    }
                        break;
                    case ' ':
                        parser->setMark(100);
                        break;
                }
            }
    //        usleep(100000);
        }
    }
    parser->stop();
    closelog();
    delete parser;
    if(reader) delete reader;
    if(logger) delete logger;
    if(sets) delete sets;
    
    return 0;
}

