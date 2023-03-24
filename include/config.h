#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #include <Preferences.h>
    #define debugSerial Serial
    #define modbusSerial Serial2
    #define DEBUG

    #define PIN_1P_IN 33
    #define PIN_1P_OUT 26
    #define PIN_3P_IN 25
    #define PIN_3P_OUT 27

    class Config{
        private:
            Preferences *_prefs;
            uint32_t _switchDelay;
        public:
            Config();
            void begin(Preferences *prefs);
            uint32_t getSwitchDelay();
            void setSwitchDelay(uint32_t value);
    };

    #ifdef DEBUG
    #define dbg(x...) debugSerial.print(x);
    #define dbgln(x...) debugSerial.println(x);
    #else /* DEBUG */
    #define dbg(x...) ;
    #define dbgln(x...) ;
    #endif /* DEBUG */
#endif /* CONFIG_H */