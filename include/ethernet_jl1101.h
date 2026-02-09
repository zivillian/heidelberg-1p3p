#pragma once

#include <stdint.h>

bool setupEthernet();
bool ethernetHasIp();
bool ethernetWaitForIp(uint32_t timeout_ms);
