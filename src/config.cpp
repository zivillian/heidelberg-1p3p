#include "config.h"

Config::Config()
    :_prefs(NULL)
    ,_switchDelay(120000)
{}

void Config::begin(Preferences *prefs)
{
    _prefs = prefs;
    _switchDelay = _prefs->getULong("switchDelay", _switchDelay);
}

uint32_t Config::getSwitchDelay(){
    return _switchDelay;
}

void Config::setSwitchDelay(uint32_t value){
    if (_switchDelay == value) return;
    _switchDelay = value;
    _prefs->putULong("switchDelay", _switchDelay);
}