#ifndef PAGES_H
    #define PAGES_H

    #include <WiFiManager.h>
    #include <ESPAsyncWebServer.h>
    #include <ModbusBridgeWiFi.h>
    #include <ModbusclientRTU.h>
    #include <Update.h>
    #include <uptime.h>
    #include "config.h"
    #include "switch.h"
    #include "debug.h"

    void setupPages(AsyncWebServer* server, PhaseSwitch *phaseSwitch, Config *config, WiFiManager *wm);
    void sendResponseHeader(AsyncResponseStream *response, const char *title);
    void sendResponseTrailer(AsyncResponseStream *response);
    void sendButton(AsyncResponseStream *response, const char *title, const char *action, const char *css = "");
    void sendPostButton(AsyncResponseStream *response, const char *title, const char *action);
    void sendTableRow(AsyncResponseStream *response, const char *name, float value);
    void sendTableRow(AsyncResponseStream *response, const char *name, uint16_t value);
    void sendTableRow(AsyncResponseStream *response, const char *name, uint32_t value);
    void sendTableRow(AsyncResponseStream *response, const char *name, String value);
    void sendTableRow(AsyncResponseStream *response, const char *name, const char *value);
    void sendDebugForm(AsyncResponseStream *response, String slaveId, String reg, String function, String count);
    const String ErrorName(Modbus::Error code);
    const String WiFiQuality(int rssiValue);
    const String ChargingState(uint16_t state);
    const String Uptime();
#endif /* PAGES_H */