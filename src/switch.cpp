#include "switch.h"

PhaseSwitch::PhaseSwitch()
  :_previous(0)
  ,_delay(0)
  ,_state(State::WaitingForOff)
  ,_desiredPhases(1)
  ,_switchingSupported(false)
  ,_inputRegister(19)
  ,_holdingRegister(7)
  ,_switchDelay(120000)
  ,_client()
  ,_bridge()
  ,_serverId(1)
{}

void PhaseSwitch::begin(){
  pinMode(PIN_1P_OUT, OUTPUT);
  digitalWrite(PIN_1P_OUT, HIGH);
  pinMode(PIN_3P_OUT, OUTPUT);
  digitalWrite(PIN_3P_OUT, HIGH);
  pinMode(PIN_1P_IN, INPUT_PULLUP);
  pinMode(PIN_3P_IN, INPUT_PULLUP);
}

void PhaseSwitch::beginModbus(){
  RTUutils::prepareHardwareSerial(modbusSerial);
  modbusSerial.begin(19200, SERIAL_8E1);
  _client.setTimeout(1000);
  _client.begin(modbusSerial);
  _bridge.attachServer(_serverId, _serverId, ANY_FUNCTION_CODE, &_client);
  _bridgeWorker = _bridge.getWorker(_serverId, ANY_FUNCTION_CODE);
  _bridge.start(502, 10, 30000);
}

void PhaseSwitch::loop(){
  if (_delay > 0){
    if (millis() - _previous < _delay){
      return;
    }
    _delay = 0;
  }
  if (_state == State::WaitingForOff){
    if (digitalRead(PIN_1P_IN) == LOW && digitalRead(PIN_3P_IN) == LOW){
      dbgln("confirmed off");
      _switchingSupported = true;
      _state = State::ConfirmedOff;
      _previous = millis();
      _delay = 2000;//todo configurable delay
    }
    return;
  }
  else if (_state == State::ConfirmedOff){
    //4. nach validierter Ladeunterbrechung das L2L3-Schütz entsprechend an oder ausgeschaltet wird,
    if (_desiredPhases == 3){
      dbgln("switching on 3p");
      digitalWrite(PIN_3P_OUT, LOW);
      _state = State::SwitchedOn;
    }
    else{
      dbgln("switching on 1p");
      digitalWrite(PIN_1P_OUT, LOW);
      _state = State::SwitchedOn;
    }
    return;
  }
  if (!validateSetup()) {
    dbgln("setup validation failed");
    _previous = millis();
    _delay = 10000;
    return;
  }
  if (_state == State::Running) {
    return;
  }
  else if (_state == State::SwitchPhases){
    //1. der aktuelle Ladevorgang unterbrochen wird (Register 261 auf 0),
    auto response = cacheWriteHolding(ModbusMessage(_serverId, WRITE_HOLD_REGISTER, HEC_REG_MAX_CURRENT, (uint16_t)0));
    if (response.getError() != SUCCESS) return;
    //2. die Box gesperrt wird (RemoteLock-Register 259 auf 0). Vielleicht reicht das auch schon alleine ohne 1.
    //   Dabei wird auch die PWM komplett abgeschaltet - wie Disconnect,
    response = cacheWriteHolding(ModbusMessage(_serverId, WRITE_HOLD_REGISTER, HEC_REG_REMOTE_LOCK, (uint16_t)0));
    if (response.getError() != SUCCESS) return;
    dbgln("phase switch started");
    _state = State::WaitingForZero;
    return;
  }
  else if (_state == State::WaitingForZero){
    //3. dann über den Status (Register 5 Status=F) und die Phasenströme (alle 0,0A) geprüft wird ob die Ladung auch definitiv beendet wurde,
    auto response = cacheReadInput(ModbusMessage(_serverId, READ_INPUT_REGISTER, (uint16_t)5, (uint16_t)4));
    if (response.getError() != SUCCESS) return;
    if (response.size() < 11) return;
    uint16_t state, l1, l2, l3;
    response.get(3, state);
    response.get(5, l1);
    response.get(7, l2);
    response.get(9, l3);
    if (state != 10) return;
    if (l1 > 0) return;
    if (l2 > 0) return;
    if (l3 > 0) return;
    dbgln("zero load confirmed");
    _state = State::ConfirmedZero;
    return;
  }
  else if (_state == State::ConfirmedZero){
    digitalWrite(PIN_1P_OUT, HIGH);
    digitalWrite(PIN_3P_OUT, HIGH);
    _state = State::WaitingForOff;
    dbgln("switched off");
    return;
  }
  else if (_state == State::SwitchedOn){
    //5. die gewünschte Zielposition des Schütz über den Hilfskontakt und die Phasenspannungsregister (>=208V) geprüft wird,
    if (_desiredPhases == 3){
      if (digitalRead(PIN_3P_IN) != HIGH) return;
      if (getActivePhases() != 3) {
        _previous = millis();
        _delay = 1000;
        return;
      }
      dbgln("confirmed 3p");
    }
    else {
      if (digitalRead(PIN_1P_IN) != HIGH) return;
      if (getActivePhases() != 1) {
        _previous = millis();
        _delay = 1000;
        return;
      }
      dbgln("confirmed 1p");
    }
    _state = State::Delay;
    //6. ein einstellbarer Zeitraum lang gewartet wird (Default 120 Sekunden),
    _previous = millis();
    _delay = _switchDelay;
    return;
  }
  else if (_state == State::Delay){
    //7. das RemoteLock-Register wieder auf 1 gesetzt,
    auto response = cacheWriteHolding(ModbusMessage(_serverId, WRITE_HOLD_REGISTER, HEC_REG_REMOTE_LOCK, 1));
    if (response.getError() != SUCCESS) return;
    //8. und dann der Ladevorgang wieder fortgesetzt wird (Register 261 auf den zuletzt von evcc gesendeten Wert, caching).
    response = cacheWriteHolding(ModbusMessage(_serverId, WRITE_HOLD_REGISTER, HEC_REG_MAX_CURRENT, _holdingRegister[HEC_REG_MAX_CURRENT - HOLDING_REG_OFFSET]));
    if (response.getError() != SUCCESS) return;
    _state = State::Running;
      dbgln("restored registers");
    return;
  }
}

void PhaseSwitch::switchTo1P(){
  if (!canSwitchTo1P()) return;
  _desiredPhases = 1;
}

void PhaseSwitch::switchTo3P(){
  if (!canSwitchTo3P()) return;
  _desiredPhases = 3;
}

bool PhaseSwitch::canSwitchTo1P(){
  return _switchingSupported
    && _firmwareSupported
    && _state == State::Running
    && _desiredPhases == 3;
}

bool PhaseSwitch::canSwitchTo3P(){
  return _switchingSupported
    && _firmwareSupported
    && _state == State::Running
    && _desiredPhases == 1;
}

void PhaseSwitch::setSwitchDelay(uint32_t millis){
  _switchDelay = millis;
}

uint32_t PhaseSwitch::getRtuMessageCount(){
  return _client.getMessageCount();
}

uint32_t PhaseSwitch::getRtuPendingRequestCount(){
  return _client.pendingRequests();
}

uint32_t PhaseSwitch::getRtuErrorCount(){
  return _client.getErrorCount();
}

uint32_t PhaseSwitch::getBridgeMessageCount(){
  return _bridge.getMessageCount();
}

uint32_t PhaseSwitch::getBridgeActiveClientCount(){
  return _bridge.activeClients();
}

uint32_t PhaseSwitch::getBridgeErrorCount(){
  return _bridge.getErrorCount();
}

ModbusMessage PhaseSwitch::sendRtuRequest(uint8_t serverID, uint8_t functionCode, uint16_t p1, uint16_t p2){
  return _client.syncRequest(0xdeadbeef, serverID, functionCode, p1, p2);
}

bool PhaseSwitch::validateSetup(){
  if (_switchingSupported && _firmwareSupported) return true;
  if (_switchingSupported){
    //check firmware
    cacheReadInput(ModbusMessage(_serverId, READ_INPUT_REGISTER, (uint16_t)4, (uint16_t)1));
    if (_inputRegister[4] == 0x108){
      //setup modbus handler
      _bridge.registerWorker(_serverId, WRITE_HOLD_REGISTER, [this](ModbusMessage msg){ return this->onWriteHolding(msg); });
      _bridge.registerWorker(_serverId, WRITE_MULT_REGISTERS, [this](ModbusMessage msg){ return this->onWriteMultiple(msg); });
      _bridge.registerWorker(_serverId, READ_HOLD_REGISTER, [this](ModbusMessage msg){ return this->onReadHolding(msg); });
      _bridge.registerWorker(_serverId, READ_INPUT_REGISTER, [this](ModbusMessage msg){ return this->onReadInput(msg); });
      //initialize caches
      auto response = cacheReadInput(ModbusMessage(_serverId, READ_INPUT_REGISTER, (uint16_t)0, (uint16_t)19));
      if (response.getError() != SUCCESS) return false;
      response = cacheReadHolding(ModbusMessage(_serverId, READ_HOLD_REGISTER, HOLDING_REG_OFFSET, (uint16_t)_holdingRegister.size()));
      if (response.getError() != SUCCESS) return false;
      _firmwareSupported = true;
    }
  }
  return _switchingSupported && _firmwareSupported;
}

uint8_t PhaseSwitch::getActivePhases(){
  auto response = cacheReadInput(ModbusMessage(_serverId, READ_INPUT_REGISTER, (uint16_t)10, (uint16_t)3));
  if (response.getError() == SUCCESS){
    uint16_t l1, l2, l3;
    response.get(3, l1);
    response.get(5, l2);
    response.get(7, l3);
    if (l1 >= 208 && l2 < 208 && l3 < 208){
      return 1;
    }
    if (l1 >= 208 && l2 >= 208 && l3 >= 208){
      return 3;
    }
  }
  return 0;
}

ModbusMessage PhaseSwitch::onWriteHolding(ModbusMessage msg){
  uint16_t addr = 0;
  uint16_t value = 0;
  msg.get(2, addr);
  msg.get(4, value);
  if (_switchingSupported && _firmwareSupported && addr == 4)
  {
    if (value == 1){
      switchTo1P();
    }
    else if (value == 3){
      switchTo3P();
    }
    else{
      return ModbusMessage(msg.getServerID(),  msg.getFunctionCode(), ILLEGAL_DATA_VALUE);
    }
    return ModbusMessage(msg.getServerID(),  msg.getFunctionCode(), addr, value);    
  }
  if (_state != State::Running){
    //write to cached registers
    if (addr < HOLDING_REG_OFFSET || addr - HOLDING_REG_OFFSET >= _holdingRegister.size()){
      return  ModbusMessage(msg.getServerID(), msg.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    }
    //Die letzte Stromänderung (Register 261) muss auch währenddessen bei Abfrage durch evcc stets wieder zurückgegeben werden (caching).
    _holdingRegister[addr - HOLDING_REG_OFFSET] = value;
    return ModbusMessage(msg.getServerID(), msg.getFunctionCode(), addr, value);
  }
  return cacheWriteHolding(msg);
}

ModbusMessage PhaseSwitch::cacheWriteHolding(ModbusMessage msg){
  auto response = _bridgeWorker(msg);
  if (response.getError() == SUCCESS){
    uint16_t addr = 0;
    uint16_t value = 0;
    response.get(2, addr);
    response.get(4, value);
    uint16_t index = 7;
    if (addr >= HOLDING_REG_OFFSET){
      addr -= HOLDING_REG_OFFSET;
      if (addr < _holdingRegister.size()){
        _holdingRegister[addr] = value;
      }
    }
  }
  return response;
}

ModbusMessage PhaseSwitch::onWriteMultiple(ModbusMessage msg){
  uint16_t addr = 0;
  uint16_t words = 0;
  msg.get(2, addr);
  msg.get(4, words);
  if (_state != State::Running){
    //write to cached registers
    if (addr < HOLDING_REG_OFFSET || addr - HOLDING_REG_OFFSET + words > _holdingRegister.size()){
      return  ModbusMessage(msg.getServerID(), msg.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    }
    //Die letzte Stromänderung (Register 261) muss auch währenddessen bei Abfrage durch evcc stets wieder zurückgegeben werden (caching).
    uint16_t index = 7;
    addr -= HOLDING_REG_OFFSET;
    for(size_t i = 0; i  < words; i++){
      uint16_t value;
      index = msg.get(index, value);
      _holdingRegister[addr] = value;
    }
    return ModbusMessage(msg.getServerID(), msg.getFunctionCode(), addr + HOLDING_REG_OFFSET, words);

  }
  return cacheWriteMultiple(msg);
}

ModbusMessage PhaseSwitch::cacheWriteMultiple(ModbusMessage msg){
  auto response = _bridgeWorker(msg);
  if (response.getError() == SUCCESS){
    uint16_t addr = 0;
    uint16_t words = 0;
    msg.get(2, addr);
    msg.get(4, words);
    uint16_t index = 7;
    addr -= HOLDING_REG_OFFSET;
    if (addr + words <= _holdingRegister.size()){
      for(uint16_t i = addr; i < addr + words; i++){
        uint16_t value;
        index = msg.get(index, value);
        _holdingRegister[i] = value;
      }
    }
  }
  return response;
}

ModbusMessage PhaseSwitch::onReadHolding(ModbusMessage msg){
  if (_switchingSupported && _firmwareSupported)
  {
    uint16_t addr = 0;
    uint16_t words = 0;
    msg.get(2, addr);
    msg.get(4, words);
    if (addr == 4 && words == 1){
      //read phase config
      ModbusMessage response;
      response.add(msg.getServerID(), msg.getFunctionCode(), (uint8_t)(words * 2));
      response.add((uint16_t)_desiredPhases);
      return response;
    }
  }
  if (_state != State::Running){
    //return cached registers
    uint16_t addr = 0;
    uint16_t words = 0;
    msg.get(2, addr);
    msg.get(4, words);
    if (addr < HOLDING_REG_OFFSET || addr - HOLDING_REG_OFFSET + words > _holdingRegister.size()){
      ModbusMessage response;
      response.setError(msg.getServerID(), msg.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
      return response;
    }
    addr -= HOLDING_REG_OFFSET;
    //RemoteLock muss währenddessen immer 1 zurückgeben (und eingehende Schreibzugriffe ignorieren).
    _holdingRegister[3] = 1;
    auto response = ModbusMessage(msg.getServerID(), msg.getFunctionCode());
    response.add((uint8_t)(words * 2));//byte count
    for(size_t i = 0; i < words; i++){
      response.add(_holdingRegister[addr + i]);
    }
    return response;
  }
  return cacheReadHolding(msg);
}

ModbusMessage PhaseSwitch::cacheReadHolding(ModbusMessage msg){
  auto response = _bridgeWorker(msg);
  if (response.getError() == SUCCESS){
    uint16_t addr = 0;
    uint16_t words = 0;
    msg.get(2, addr);
    msg.get(4, words);
    uint16_t index = 3;
    addr -= HOLDING_REG_OFFSET;
    if (addr + words <= _holdingRegister.size()){
      for(uint16_t i = addr; i < addr + words; i++){
        uint16_t value;
        index = response.get(index, value);
        _holdingRegister[i] = value;
      }
    }
  }
  return response;
}

ModbusMessage PhaseSwitch::onReadInput(ModbusMessage msg){
  if (_state != State::Running){
    //return cached registers
    uint16_t addr = 0;
    uint16_t words = 0;
    msg.get(2, addr);
    msg.get(4, words);
    if (addr < 4 || addr + words > _inputRegister.size()){
     ModbusMessage response;
     response.setError(msg.getServerID(), msg.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
     return response;
    }
    //Für evcc muss es während der Umschaltprozedur so aussehen als wäre das Fahrzeug angeschlossen aber lädt nicht (Status B).
    _inputRegister[5] = 4; //B1
    auto response = ModbusMessage(msg.getServerID(), msg.getFunctionCode());
    response.add((uint8_t)(words * 2));//byte count
    for(size_t i = 0; i < words; i++){
      response.add(_inputRegister[addr + i]);
    }
    return response;
  }
  return cacheReadInput(msg);
}

ModbusMessage PhaseSwitch::cacheReadInput(ModbusMessage msg){
  auto response = _bridgeWorker(msg);
  if (response.getError() == SUCCESS){
    uint16_t addr = 0;
    uint16_t words = 0;
    msg.get(2, addr);
    msg.get(4, words);
    uint16_t index = 3;
    if (addr + words <= _inputRegister.size()){
      for(uint16_t i = addr; i < addr + words; i++){
        uint16_t value;
        index = response.get(index, value);
        _inputRegister[i] = value;
      }
    }
  }
  return response;
}