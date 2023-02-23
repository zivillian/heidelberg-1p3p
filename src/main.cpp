#include "main.h"


ModbusMessage filterWriteHolding(ModbusMessage request){
  dbgln("filterWriteHolding");
  return MBbridgeWorker(request);
}

ModbusMessage filterReadHolding(ModbusMessage request){
  dbgln("filterReadHolding");
  return MBbridgeWorker(request);  
}

ModbusMessage filterReadInput(ModbusMessage request){
  dbgln("filterReadInput");
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
  MBbridge.attachServer(1, 1, ANY_FUNCTION_CODE, MBclient);
  MBbridgeWorker = MBbridge.getWorker(1, ANY_FUNCTION_CODE);
  MBbridge.registerWorker(1, WRITE_HOLD_REGISTER, filterWriteHolding);
  MBbridge.registerWorker(1, READ_HOLD_REGISTER, filterReadHolding);
  MBbridge.registerWorker(1, READ_INPUT_REGISTER, filterReadHolding);
  MBbridge.start(502, 10, 30000);
  dbgln("[modbus] finished");
}

void loop() {
  switch(currentState){
    case State::Running:
      return;
  }
}