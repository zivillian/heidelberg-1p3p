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
        #define PIN_ETH_MDC 23
        #define PIN_ETH_MDIO 18
        #define PIN_ETH_PWR 0
        #define PIN_ETH_CLK 17
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
            bool _ethDhcp;
            String _ethIp;
            String _ethGw;
            String _ethMask;
            String _ethDns1;
            String _ethDns2;
            bool _wifiDhcp;
            String _wifiIp;
            String _wifiGw;
            String _wifiMask;
            String _wifiDns1;
            String _wifiDns2;
            bool _wifiCredsSet;
            bool _modbusEnabled;
            String _hostname;
        public:
            Config();
            void begin(Preferences *prefs);
            uint32_t getSwitchDelay();
            void setSwitchDelay(uint32_t value);
            bool getEthDhcp();
            void setEthDhcp(bool value);
            String getEthIp();
            void setEthIp(String value);
            String getEthGw();
            void setEthGw(String value);
            String getEthMask();
            void setEthMask(String value);
            String getEthDns1();
            void setEthDns1(String value);
            String getEthDns2();
            void setEthDns2(String value);
            bool getWifiDhcp();
            void setWifiDhcp(bool value);
            String getWifiIp();
            void setWifiIp(String value);
            String getWifiGw();
            void setWifiGw(String value);
            String getWifiMask();
            void setWifiMask(String value);
            String getWifiDns1();
            void setWifiDns1(String value);
            String getWifiDns2();
            void setWifiDns2(String value);
            bool getWifiCredsSet();
            void setWifiCredsSet(bool value);
            bool getModbusEnabled();
            void setModbusEnabled(bool value);
            String getHostname();
            void setHostname(String value);
            static bool isHostnameValid(const String &value);
    };

#endif /* CONFIG_H */
