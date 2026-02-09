#include "main.h"
#include "ethernet_jl1101.h"

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
  if (!config.getWifiDhcp()) {
    IPAddress ip;
    IPAddress gw;
    IPAddress mask;
    IPAddress dns1;
    IPAddress dns2;
    ip.fromString(config.getWifiIp());
    gw.fromString(config.getWifiGw());
    mask.fromString(config.getWifiMask());
    dns1.fromString(config.getWifiDns1());
    dns2.fromString(config.getWifiDns2());
    WiFi.config(ip, gw, mask, dns1, dns2);
  }

#ifdef BOARD_DINGTIAN
  setupEthernet();
  if (config.getEthDhcp()) {
    ethernetConfigureDhcp();
  } else {
    IPAddress ip;
    IPAddress gw;
    IPAddress mask;
    IPAddress dns1;
    IPAddress dns2;
    ip.fromString(config.getEthIp());
    gw.fromString(config.getEthGw());
    mask.fromString(config.getEthMask());
    dns1.fromString(config.getEthDns1());
    dns2.fromString(config.getEthDns2());
    ethernetConfigureStatic(ip, gw, mask, dns1, dns2);
  }
  const bool eth_ok = ethernetWaitForIp(5000);
#endif
  
#ifdef BOARD_DINGTIAN
  debugOut.begin(23, false);
#endif
  wm.setDebugOutput(false);

  pinMode(PIN_FACTORY_LED, OUTPUT);
  digitalWrite(PIN_FACTORY_LED, LOW);

  wm.setClass("invert");
  auto reboot = false;
  wm.setAPCallback([&reboot](WiFiManager *wifiManager){reboot = true;});
#ifdef BOARD_DINGTIAN
  if (!eth_ok) {
    wm.autoConnect();
  }
#else
  wm.autoConnect();
#endif
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
