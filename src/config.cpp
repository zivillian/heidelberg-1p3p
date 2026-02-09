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
{}

void Config::begin(Preferences *prefs)
{
    _prefs = prefs;
    _switchDelay = _prefs->getULong("switchDelay", _switchDelay);
    _ethDhcp = _prefs->getBool("ethDhcp", _ethDhcp);
    _ethIp = _prefs->getString("ethIp", _ethIp);
    _ethGw = _prefs->getString("ethGw", _ethGw);
    _ethMask = _prefs->getString("ethMask", _ethMask);
    _ethDns1 = _prefs->getString("ethDns1", _ethDns1);
    _ethDns2 = _prefs->getString("ethDns2", _ethDns2);
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
