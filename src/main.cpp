#include "main.h"


ModbusMessage filterWriteHolding(ModbusMessage request){
  dbgln("filterWriteHolding");
  if (currentState != Running)
  {
    return NIL_RESPONSE;
  }
  uint16_t addr = 0;
  uint16_t words = 0;
  request.get(2, addr);
  request.get(4, words);
  dbgln(addr);
  dbgln(words);
  return MBbridgeWorker(request);
}

ModbusMessage filterReadHolding(ModbusMessage request){
  dbgln("filterReadHolding");
  if (currentState != Running)
  {
    return NIL_RESPONSE;
  }
  uint16_t addr = 0;
  uint16_t words = 0;
  request.get(2, addr);
  request.get(4, words);
  if (addr == 4 && words == 1){
    //read phase config
    ModbusMessage response;
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
    response.add((uint16_t)desiredPhases);
    return response;
  }
  return MBbridgeWorker(request);
}

ModbusMessage filterReadInput(ModbusMessage request){
  dbgln("filterReadInput");
  if (currentState != Running)
  {
    return NIL_RESPONSE;
  }
  return MBbridgeWorker(request);
}

void setup() {
  debugSerial.begin(115200);
  dbgln("[wifi] start");
  WiFi.mode(WIFI_STA);
  wm.setClass("invert");
  wm.autoConnect();
  dbgln("[wifi] finished");
  dbgln("[modbus] start");
  MBclient = new ModbusClientTCPasync({192, 168, 23, 113});
  MBclient->setTimeout(1000);
  MBbridge.attachServer(serverId, serverId, ANY_FUNCTION_CODE, MBclient);
  MBbridgeWorker = MBbridge.getWorker(serverId, ANY_FUNCTION_CODE);
  MBbridge.registerWorker(serverId, WRITE_HOLD_REGISTER, filterWriteHolding);
  MBbridge.registerWorker(serverId, READ_HOLD_REGISTER, filterReadHolding);
  MBbridge.registerWorker(serverId, READ_INPUT_REGISTER, filterReadHolding);
  MBbridge.start(502, 10, 30000);
  dbgln("[modbus] finished");
  currentState = Running;
}

void loop() {
  switch(currentState){
    case State::Running:
      return;
  }
}