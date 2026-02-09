#include "config.h"

Config::Config()
    :_prefs(NULL)
    ,_switchDelay(120000)
    ,_ethDhcp(true)
    ,_ethIp("192.168.178.200")
    ,_ethGw("192.168.178.1")
    ,_ethMask("255.255.255.0")
    ,_ethDns1("192.168.178.1")
    ,_ethDns2("")
    ,_wifiDhcp(true)
    ,_wifiIp("192.168.178.201")
    ,_wifiGw("192.168.178.1")
    ,_wifiMask("255.255.255.0")
    ,_wifiDns1("192.168.178.1")
    ,_wifiDns2("")
    ,_modbusEnabled(true)
{}

void Config::begin(Preferences *prefs)
{
    _prefs = prefs;
    _switchDelay = _prefs->getULong("switchDelay", _switchDelay);
    if (_prefs->isKey("ethDhcp")) _ethDhcp = _prefs->getBool("ethDhcp", _ethDhcp);
    if (_prefs->isKey("ethIp")) _ethIp = _prefs->getString("ethIp", _ethIp);
    if (_prefs->isKey("ethGw")) _ethGw = _prefs->getString("ethGw", _ethGw);
    if (_prefs->isKey("ethMask")) _ethMask = _prefs->getString("ethMask", _ethMask);
    if (_prefs->isKey("ethDns1")) _ethDns1 = _prefs->getString("ethDns1", _ethDns1);
    if (_prefs->isKey("ethDns2")) _ethDns2 = _prefs->getString("ethDns2", _ethDns2);
    if (_prefs->isKey("wifiDhcp")) _wifiDhcp = _prefs->getBool("wifiDhcp", _wifiDhcp);
    if (_prefs->isKey("wifiIp")) _wifiIp = _prefs->getString("wifiIp", _wifiIp);
    if (_prefs->isKey("wifiGw")) _wifiGw = _prefs->getString("wifiGw", _wifiGw);
    if (_prefs->isKey("wifiMask")) _wifiMask = _prefs->getString("wifiMask", _wifiMask);
    if (_prefs->isKey("wifiDns1")) _wifiDns1 = _prefs->getString("wifiDns1", _wifiDns1);
    if (_prefs->isKey("wifiDns2")) _wifiDns2 = _prefs->getString("wifiDns2", _wifiDns2);
    if (_prefs->isKey("modbusEnabled")) _modbusEnabled = _prefs->getBool("modbusEnabled", _modbusEnabled);
}

uint32_t Config::getSwitchDelay(){
    return _switchDelay;
}

void Config::setSwitchDelay(uint32_t value){
    if (_switchDelay == value) return;
    _switchDelay = value;
    _prefs->putULong("switchDelay", _switchDelay);
}

bool Config::getEthDhcp(){
    return _ethDhcp;
}

void Config::setEthDhcp(bool value){
    if (_ethDhcp == value) return;
    _ethDhcp = value;
    _prefs->putBool("ethDhcp", _ethDhcp);
}

String Config::getEthIp(){
    return _ethIp;
}

void Config::setEthIp(String value){
    if (_ethIp == value) return;
    _ethIp = value;
    _prefs->putString("ethIp", _ethIp);
}

String Config::getEthGw(){
    return _ethGw;
}

void Config::setEthGw(String value){
    if (_ethGw == value) return;
    _ethGw = value;
    _prefs->putString("ethGw", _ethGw);
}

String Config::getEthMask(){
    return _ethMask;
}

void Config::setEthMask(String value){
    if (_ethMask == value) return;
    _ethMask = value;
    _prefs->putString("ethMask", _ethMask);
}

String Config::getEthDns1(){
    return _ethDns1;
}

void Config::setEthDns1(String value){
    if (_ethDns1 == value) return;
    _ethDns1 = value;
    _prefs->putString("ethDns1", _ethDns1);
}

String Config::getEthDns2(){
    return _ethDns2;
}

void Config::setEthDns2(String value){
    if (_ethDns2 == value) return;
    _ethDns2 = value;
    _prefs->putString("ethDns2", _ethDns2);
}

bool Config::getWifiDhcp(){
    return _wifiDhcp;
}

void Config::setWifiDhcp(bool value){
    if (_wifiDhcp == value) return;
    _wifiDhcp = value;
    _prefs->putBool("wifiDhcp", _wifiDhcp);
}

String Config::getWifiIp(){
    return _wifiIp;
}

void Config::setWifiIp(String value){
    if (_wifiIp == value) return;
    _wifiIp = value;
    _prefs->putString("wifiIp", _wifiIp);
}

String Config::getWifiGw(){
    return _wifiGw;
}

void Config::setWifiGw(String value){
    if (_wifiGw == value) return;
    _wifiGw = value;
    _prefs->putString("wifiGw", _wifiGw);
}

String Config::getWifiMask(){
    return _wifiMask;
}

void Config::setWifiMask(String value){
    if (_wifiMask == value) return;
    _wifiMask = value;
    _prefs->putString("wifiMask", _wifiMask);
}

String Config::getWifiDns1(){
    return _wifiDns1;
}

void Config::setWifiDns1(String value){
    if (_wifiDns1 == value) return;
    _wifiDns1 = value;
    _prefs->putString("wifiDns1", _wifiDns1);
}

String Config::getWifiDns2(){
    return _wifiDns2;
}

void Config::setWifiDns2(String value){
    if (_wifiDns2 == value) return;
    _wifiDns2 = value;
    _prefs->putString("wifiDns2", _wifiDns2);
}

bool Config::getModbusEnabled(){
    return _modbusEnabled;
}

void Config::setModbusEnabled(bool value){
    if (_modbusEnabled == value) return;
    _modbusEnabled = value;
    _prefs->putBool("modbusEnabled", _modbusEnabled);
}
