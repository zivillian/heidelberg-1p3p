#include "main.h"

AsyncWebServer webServer(80);
Config config;
Preferences prefs;
PhaseSwitch phaseSwitch;
TelnetPrint telnet;
WiFiManager wm(telnet);

void setup() {
  dbgln("[gpio] start");
  phaseSwitch.begin();
  dbgln("[gpio] finished");
  dbgln("[config] load")
  prefs.begin("hec_1p3p");
  config.begin(&prefs);
  phaseSwitch.setSwitchDelay(config.getSwitchDelay());
  dbgln("[wifi] start");
  WiFi.mode(WIFI_STA);
  wm.setDebugOutput(false);
  wm.setClass("invert");
  wm.autoConnect();
  telnet.begin(23, false);
  pinMode(PIN_FACTORY_LED, OUTPUT);
  pinMode(PIN_FACTORY_BTN, INPUT);
  digitalWrite(PIN_FACTORY_LED, LOW);
  LOGDEVICE = &telnet;
  dbgln("[wifi] finished");
  dbgln("[modbus] start");
  phaseSwitch.beginModbus();
  dbgln("[modbus] finished");
  setupPages(&webServer, &phaseSwitch, &config, &wm);
  webServer.begin();
  dbgln("[setup] finished");
}

int last = 0;

void loop() {
  uptime::calculateUptime();
  telnet.loop();
  phaseSwitch.loop();
  if (millis() - last > 2000){
    last = millis();
    dbgln(digitalRead(PIN_FACTORY_BTN))
    dbgln(digitalRead(PIN_1P_IN))
  }
}