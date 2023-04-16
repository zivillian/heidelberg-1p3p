#include "main.h"

AsyncWebServer webServer(80);
Config config;
Preferences prefs;
PhaseSwitch phaseSwitch;
#ifdef BOARD_DINGTIAN
TelnetPrint debugOut;
#endif
WiFiManager wm(debugOut);

void setup() {
#ifdef BOARD_DINGTIAN
  debugOut.begin(23, false);
#else
  debugOut.begin(115200);
#endif
  dbgln("[gpio] start");
  phaseSwitch.begin();
  dbgln("[gpio] finished");
  dbgln("[config] load")
  prefs.begin("hec_1p3p");
  config.begin(&prefs);
  phaseSwitch.setSwitchDelay(config.getSwitchDelay());
  dbgln("[wifi] start");
  WiFi.mode(WIFI_STA);
  wm.setClass("invert");
  wm.autoConnect();
  LOGDEVICE = &debugOut;
  dbgln("[wifi] finished");
  dbgln("[modbus] start");
  phaseSwitch.beginModbus();
  dbgln("[modbus] finished");
  setupPages(&webServer, &phaseSwitch, &config, &wm);
  webServer.begin();
  dbgln("[setup] finished");
}

void loop() {
  uptime::calculateUptime();
#ifdef BOARD_DINGTIAN
  debugOut.loop();
#endif
  phaseSwitch.loop();
}