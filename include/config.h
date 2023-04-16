#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #include <Preferences.h>

    #ifdef BOARD_DINGTIAN
        #define debugOut telnet
        #define modbusSerial Serial
        #define PIN_1P_IN 36
        #define PIN_1P_OUT 16
        #define PIN_3P_IN 39
        #define PIN_3P_OUT 2
        #define PIN_RS485_DE 33
        #define PIN_FACTORY_LED 32
        #define PIN_FACTORY_BTN 34
        #define RELAY_ON HIGH
        #define RELAY_OFF LOW
    #else
        #define debugOut Serial
        #define modbusSerial Serial2

        #define PIN_1P_IN 33
        #define PIN_1P_OUT 26
        #define PIN_3P_IN 25
        #define PIN_3P_OUT 27
        #define PIN_RS485_DE -1
        #define RELAY_ON LOW
        #define RELAY_OFF HIGH
    #endif

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

#endif /* CONFIG_H */