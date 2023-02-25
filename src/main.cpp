#include "main.h"
#include "pages.h"

ModbusMessage filterWriteHolding(ModbusMessage request){
  dbgln("filterWriteHolding");
  if (currentState.CurrentState != Running)
  {
    return NIL_RESPONSE;
  }
  if (switchingSupported)
  {
    uint16_t addr = 0;
    uint16_t phases = 0;
    request.get(2, addr);
    request.get(4, phases);
    if (addr == 4){
      if (phases != 1 && phases != 3){
        return ModbusMessage(request.getServerID(), WRITE_HOLD_REGISTER, ILLEGAL_DATA_VALUE);
      }
      if (currentState.DesiredPhases != phases){
        currentState.DesiredPhases = phases;
        currentState.CurrentState = State::SwitchPhases;
      }
      return ModbusMessage(request.getServerID(), WRITE_HOLD_REGISTER, (uint16_t)4, (uint16_t)currentState.DesiredPhases);
    }
  }
  return MBbridgeWorker(request);
}

ModbusMessage filterReadHolding(ModbusMessage request){
  dbgln("filterReadHolding");
  if (currentState.CurrentState != Running)
  {
    return NIL_RESPONSE;
  }
  if (switchingSupported)
  {
    uint16_t addr = 0;
    uint16_t words = 0;
    request.get(2, addr);
    request.get(4, words);
    if (addr == 4 && words == 1){
      //read phase config
      ModbusMessage response;
      response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
      response.add((uint16_t)currentState.DesiredPhases);
      return response;
    }
  }
  return MBbridgeWorker(request);
}

void readCurrent(){
  MBclient->addRequest(MODBUS_REQUEST_TOKEN, serverId, READ_INPUT_REGISTER, (uint16_t)5, (uint16_t)4);
  dbgln("requested Current");
}

void clientResponse(ModbusMessage msg, uint32_t token){
  dbgln("clientResponse");
  if (currentState.CurrentState != WaitingForZero) return;
  if (token != MODBUS_REQUEST_TOKEN) return;
  if (msg.getError() != SUCCESS){
    dbgln("error 2");
    readCurrent();
    return;
  }
  if (msg.getFunctionCode() != READ_INPUT_REGISTER){
    readCurrent();
    return;
  }
  uint8_t bytes = msg[2];
  if (bytes != 8){
    readCurrent();
    return;
  }
  uint16_t state = 0;
  uint16_t l1 = 0;
  uint16_t l2 = 0;
  uint16_t l3 = 0;
  msg.get(3, state);
  msg.get(5, l1);
  msg.get(7, l2);
  msg.get(9, l3);
  if (state != 2 && state != 4 && state != 6){
    readCurrent();
    return;
  }
  if (l1 != 0 || l2 != 0 && l3 != 0){
    readCurrent();
    return;
  }
  currentState.CurrentState = State::ConfirmedZero;
}

void setup() {
  debugSerial.begin(115200);
  dbgln("[gpio] start");
  pinMode(PIN_1P_OUT, OUTPUT);
  digitalWrite(PIN_1P_OUT, LOW);
  pinMode(PIN_3P_OUT, OUTPUT);
  digitalWrite(PIN_3P_OUT, LOW);
  pinMode(PIN_1P_IN, INPUT_PULLUP);
  pinMode(PIN_3P_IN, INPUT_PULLUP);
  dbgln("[gpio] finished");
  dbgln("[wifi] start");
  WiFi.mode(WIFI_STA);
  wm.setClass("invert");
  wm.autoConnect();
  dbgln("[wifi] finished");
  dbgln("[modbus] start");
  MBclient = new ModbusClientTCPasync({192, 168, 23, 87});
  MBclient->setTimeout(30000);
  MBclient->onResponseHandler(clientResponse);
  MBbridge.attachServer(serverId, serverId, ANY_FUNCTION_CODE, MBclient);
  MBbridgeWorker = MBbridge.getWorker(serverId, ANY_FUNCTION_CODE);
  MBbridge.registerWorker(serverId, WRITE_HOLD_REGISTER, filterWriteHolding);
  MBbridge.registerWorker(serverId, READ_HOLD_REGISTER, filterReadHolding);
  MBbridge.start(502, 10, 30000);
  dbgln("[modbus] finished");
  setupPages(&webServer, MBclient, &MBbridge, &currentState, &wm);
  webServer.begin();
  dbgln("[setup] finished");
}

void loop() {
  if (!switchingSupported) {
    if (digitalRead(PIN_1P_IN) == LOW && digitalRead(PIN_3P_IN) == LOW){
      dbgln("detected switch support");
      switchingSupported = true;
    }
    return;
  }
  switch(currentState.CurrentState){
    case State::Running:
      break;
    case State::SwitchPhases:
      dbgln("phase switch requested");
      if ((currentState.DesiredPhases == 3 && digitalRead(PIN_3P_IN) == LOW)
          || (currentState.DesiredPhases == 1 && digitalRead(PIN_1P_IN) == LOW)){
        dbgln("switching phases");
        currentState.CurrentState = State::WaitingForZero;
        MBclient->addRequest(MODBUS_REQUEST_TOKEN + 1, serverId, WRITE_HOLD_REGISTER, (uint16_t)261, (uint16_t)0);
        readCurrent();
      }
      else {
        currentState.CurrentState = State::Running;
      }
      break;
    case State::WaitingForZero:
      break;
    case State::ConfirmedZero:
      dbgln("confirmed zero");
      digitalWrite(PIN_1P_OUT, LOW);
      digitalWrite(PIN_3P_OUT, LOW);
      currentState.CurrentState = State::WaitingForOff;
      break;
    case State::WaitingForOff:
      if (digitalRead(PIN_1P_IN) == LOW && digitalRead(PIN_3P_IN) == LOW){
        dbgln("confirmed off");
        currentState.CurrentState = State::ConfirmedOff;
        waitStarted = millis();
      }
      break;;
    case State::ConfirmedOff:
      if (millis() - waitStarted >= 2000){
        switch(currentState.DesiredPhases){
          case 1:
            dbgln("switching on 1p");
            digitalWrite(PIN_1P_OUT, HIGH);
            currentState.CurrentState = State::SwitchedOn;
            break;
          case 3:
            dbgln("switching on 3p");
            digitalWrite(PIN_3P_OUT, HIGH);
            currentState.CurrentState = State::SwitchedOn;
            break;
        }
      }
      break;
    case State::SwitchedOn:
      switch (currentState.DesiredPhases){
        case 1:
          if (digitalRead(PIN_1P_IN) == HIGH){
            dbgln("confirmed 1p");
            currentState.CurrentState = State::Running;
          }
          break;
        case 3:
          if (digitalRead(PIN_3P_IN) == HIGH){
            dbgln("confirmed 3p");
            currentState.CurrentState = State::Running;
          }
          break;
      }
      break;
  }
}