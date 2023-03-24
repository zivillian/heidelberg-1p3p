#ifndef MAIN_H
    #define MAIN_H
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <WiFiManager.h>
    #include <ESPAsyncWebServer.h>
    #include <esp_task_wdt.h>
    #include <Logging.h>
    #include "config.h"
    #include "switch.h"
    #include "pages.h"

    WiFiManager wm;
    AsyncWebServer webServer(80);
    Config config;
    Preferences prefs;
    PhaseSwitch phaseSwitch;
#endif /* MAIN_H */