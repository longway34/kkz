/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: longway
 *
 * Created on 23 июля 2016 г., 22:45
 */

#include <cstdlib>
#include "WebServer.h"

using namespace std;

bool noConsole = false;

//
void sigpipe_handler(int signal){
    if(signal == SIGPIPE){
        printf("Большая нагрузка по HTTP...\n");
    }
}

/*
 * 
 */
int main(int argc, char** argv) {

    int count = 1;
    while(count < argc){
        if(!strncmp(argv[count],"-nocon",5)){
            noConsole = true;
            count++;
            continue;        
        }
    }
    
    signal(SIGPIPE, SIG_IGN);    

    while(1){
        WebServer *server = new WebServer();
    
        pthread_join(server->serverThreadId, NULL);
        
        delete server;
    }

    return 0;
}

