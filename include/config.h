#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #include <Preferences.h>
    #define modbusSerial Serial

    #define PIN_1P_IN 36
    #define PIN_1P_OUT 16
    #define PIN_3P_IN 39
    #define PIN_3P_OUT 2
    #define PIN_RS485_DE 33
    #define PIN_FACTORY_LED 32
    #define PIN_FACTORY_BTN 34

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