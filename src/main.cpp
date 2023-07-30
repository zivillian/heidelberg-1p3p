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
#ifndef BOARD_DINGTIAN
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
  
#ifdef BOARD_DINGTIAN
  debugOut.begin(23, false);
#endif
  wm.setDebugOutput(false);

  pinMode(PIN_FACTORY_LED, OUTPUT);
  digitalWrite(PIN_FACTORY_LED, LOW);

  wm.setClass("invert");
  auto reboot = false;
  wm.setAPCallback([&reboot](WiFiManager *wifiManager){reboot = true;});
  wm.autoConnect();
  if (reboot){
    ESP.restart();
  }
  MBUlogLvl = LOG_LEVEL_WARNING;
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