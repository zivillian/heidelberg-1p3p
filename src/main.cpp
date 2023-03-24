#include "main.h"

void setup() {
  debugSerial.begin(115200);
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
  dbgln("[wifi] finished");
  dbgln("[modbus] start");
  phaseSwitch.beginModbus();
  dbgln("[modbus] finished");
  setupPages(&webServer, &phaseSwitch, &config, &wm);
  webServer.begin();
  dbgln("[setup] finished");
  esp_task_wdt_init(2, true);
  esp_task_wdt_add(NULL);
}

void loop() {
  esp_task_wdt_reset();
  phaseSwitch.loop();
}