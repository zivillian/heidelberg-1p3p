#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <IPAddress.h>

bool setupEthernet();
bool ethernetHasIp();
bool ethernetHasLink();
bool ethernetWaitForIp(uint32_t timeout_ms);
String ethernetGetIpString();
String ethernetGetMacString();
bool ethernetSetHostname(const char *hostname);
bool ethernetConfigureDhcp();
bool ethernetConfigureStatic(IPAddress ip, IPAddress gw, IPAddress mask, IPAddress dns1, IPAddress dns2);
