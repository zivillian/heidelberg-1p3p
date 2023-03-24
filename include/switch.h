#ifndef SWITCH_H
    #define SWITCH_H
    #include <Arduino.h>
    #include <ModbusBridgeWiFi.h>
    #include <ModbusclientRTU.h>
    #include "config.h"

    #define HEC_REG_REMOTE_LOCK (uint16_t)259
    #define HEC_REG_MAX_CURRENT (uint16_t)261
    #define HOLDING_REG_OFFSET (uint16_t)256

    enum State {
        //wait for phase switch
        Running,
        //received request to switch phases
        SwitchPhases,    
        //sent MaxCurrent=0
        WaitingForZero, 
        //received L1-3 Current = 0
        ConfirmedZero,  
        //switched Wallbox off
        WaitingForOff,
        //confirmed all is off
        ConfirmedOff,
        //switched Wallbox on with desired phases
        SwitchedOn,
        //confirmed phases -> Wait for Delay,
        Delay,
        //delay passed -> Running
    };

    class PhaseSwitch{
        private:
            unsigned long _previous;
            unsigned long _delay;
            State _state;
            uint8_t _desiredPhases;
            bool _switchingSupported;
            bool _firmwareSupported;
            std::vector<uint16_t> _inputRegister;
            std::vector<uint16_t> _holdingRegister;
            uint32_t _switchDelay;
            bool validateSetup();
            uint8_t getActivePhases();
            //modbus
            ModbusClientRTU _client;
            ModbusBridgeWiFi _bridge;
            uint8_t _serverId;
            MBSworker _bridgeWorker;
            ModbusMessage onWriteHolding(ModbusMessage msg);
            ModbusMessage cacheWriteHolding(ModbusMessage msg);
            ModbusMessage onWriteMultiple(ModbusMessage msg);
            ModbusMessage cacheWriteMultiple(ModbusMessage msg);
            ModbusMessage onReadHolding(ModbusMessage msg);
            ModbusMessage cacheReadHolding(ModbusMessage msg);
            ModbusMessage onReadInput(ModbusMessage msg);
            ModbusMessage cacheReadInput(ModbusMessage msg);
        public:
            PhaseSwitch();
            void begin();
            void beginModbus();
            void loop();
            void switchTo1P();
            void switchTo3P();
            bool canSwitchTo1P();
            bool canSwitchTo3P();
            void setSwitchDelay(uint32_t millis);
            uint32_t getRtuMessageCount();
            uint32_t getRtuPendingRequestCount();
            uint32_t getRtuErrorCount();
            uint32_t getBridgeMessageCount();
            uint32_t getBridgeActiveClientCount();
            uint32_t getBridgeErrorCount();
            ModbusMessage sendRtuRequest(uint8_t serverID, uint8_t functionCode, uint16_t p1, uint16_t p2);
    };
#endif /* SWITCH_H */