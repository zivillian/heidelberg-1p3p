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
    
    #define PIN_1P_IN 33
    #define PIN_1P_OUT 26
    #define PIN_3P_IN 25
    #define PIN_3P_OUT 27
    #define MODBUS_REQUEST_TOKEN 3141592653
    uint8_t serverId = 1;
    bool switchingSupported = false;
    PhaseState currentState;
    unsigned long waitStarted = 0;
    ModbusClientTCPasync *MBclient;
    ModbusBridgeWiFi MBbridge;
    MBSworker MBbridgeWorker;
    WiFiManager wm;
    AsyncWebServer webServer(80);
#endif /* MAIN_H */