#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #include <Preferences.h>
    #define debugSerial Serial
    #define modbusSerial Serial2
    #define DEBUG

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
        SwitchedOn
        //confirmed phases -> Running
    };

    class PhaseState{
        public:
            PhaseState();
            State CurrentState;
            uint8_t DesiredPhases;
    };

    #ifdef DEBUG
    #define dbg(x...) debugSerial.print(x);
    #define dbgln(x...) debugSerial.println(x);
    #else /* DEBUG */
    #define dbg(x...) ;
    #define dbgln(x...) ;
    #endif /* DEBUG */
#endif /* CONFIG_H */