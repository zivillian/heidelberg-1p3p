#ifndef MAIN_H
    #define MAIN_H
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <WiFiManager.h>
    #include <ESPAsyncWebServer.h>
    #include <Logging.h>
    #include <ModbusBridgeWiFi.h>
    #include <ModbusclientTCPasync.h>
    #include "config.h"
    
    enum State {
        //wait for phase switch
        Running,        
        //sent MaxCurrent=0
        WaitingForZero, 
        //received L1-3 Current = 0
        ConfirmedZero,  
        //switched Wallbox off
        WaitingForOff,
        //confirmed all is off
        ConfirmedOff,
        //switched Wallbox on with desired phases
        SwitchedOn
        //confirmed phases -> Running
    };
    byte desiredPhases = 3;
    State currentState = State::WaitingForOff;
    unsigned long waitUntil = 0;
    ModbusClientTCPasync *MBclient;
    ModbusBridgeWiFi MBbridge;
    MBSworker MBbridgeWorker;
    WiFiManager wm;
#endif /* MAIN_H */