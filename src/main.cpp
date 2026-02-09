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

static void applyWifiConfig(Config &cfg)
{
  if (!cfg.getWifiDhcp()) {
    IPAddress ip;
    IPAddress gw;
    IPAddress mask;
    IPAddress dns1;
    IPAddress dns2;
    ip.fromString(cfg.getWifiIp());
    gw.fromString(cfg.getWifiGw());
    mask.fromString(cfg.getWifiMask());
    dns1.fromString(cfg.getWifiDns1());
    dns2.fromString(cfg.getWifiDns2());
    WiFi.config(ip, gw, mask, dns1, dns2);
  } else {
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  }
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

static void disableWifiForEthernet()
{
  WiFi.softAPdisconnect(true);
  WiFi.enableAP(false);
  WiFi.mode(WIFI_OFF);
}

static void enableWifiAfterEthernet(Config &cfg)
{
  WiFi.softAPdisconnect(true);
  WiFi.enableAP(false);
  WiFi.mode(WIFI_STA);
  applyWifiConfig(cfg);
  WiFi.begin();
}

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
  applyWifiConfig(config);

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
  if (config.getModbusEnabled()) {
    phaseSwitch.beginModbus();
    dbgln("[modbus] finished");
  } else {
    dbgln("[modbus] disabled in config");
  }
  setupPages(&webServer, &phaseSwitch, &config, &wm);
  webServer.begin();
  dbgln("[setup] finished");
}

void loop() {
  uptime::calculateUptime();
#ifdef BOARD_DINGTIAN
  debugOut.loop();
  static bool wifi_disabled_by_eth = false;
  if (ethernetHasLink() && ethernetHasIp()) {
    if (!wifi_disabled_by_eth && WiFi.getMode() != WIFI_OFF) {
      dbgln("[wifi] disabled due to ethernet");
      disableWifiForEthernet();
      wifi_disabled_by_eth = true;
    }
  } else {
    if (wifi_disabled_by_eth) {
      dbgln("[wifi] ethernet down, re-enabling wifi");
      enableWifiAfterEthernet(config);
      WiFi.reconnect();
      wifi_disabled_by_eth = false;
    }
  }
#endif
  static uint32_t wifi_no_ip_since = 0;
  static uint32_t wifi_reconnect_since = 0;
  if (WiFi.getMode() != WIFI_OFF) {
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
      if (wifi_no_ip_since == 0) {
        wifi_no_ip_since = millis();
      } else if (millis() - wifi_no_ip_since > 10000) {
        dbgln("[wifi] no IP, restarting DHCP");
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.disconnect(false, false);
        WiFi.reconnect();
        wifi_no_ip_since = 0;
      }
    } else {
      wifi_no_ip_since = 0;
    }

    if (WiFi.status() != WL_CONNECTED) {
      if (wifi_reconnect_since == 0) {
        wifi_reconnect_since = millis();
      } else if (millis() - wifi_reconnect_since > 15000) {
        dbgln("[wifi] not connected, retrying");
        applyWifiConfig(config);
        WiFi.reconnect();
        wifi_reconnect_since = 0;
      }
    } else {
      wifi_reconnect_since = 0;
    }
  } else {
    wifi_no_ip_since = 0;
    wifi_reconnect_since = 0;
  }
  delay(1);
  phaseSwitch.loop();
}
