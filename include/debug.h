#ifndef DEBUG_H
    #define DEBUG_H

    #include <Arduino.h>
    #include <ESPAsyncWebServer.h>
    class WebPrint:public Print{
        private:
            Print *_serial;
            AsyncResponseStream *_response;
            String escapeAmp(String text);
            String escapeLt(String text);
            String escape(String buffer, char oldValue, String newValue);
        public:
            WebPrint(Print *serial, AsyncResponseStream *reponse);
            size_t write(uint8_t) override;
            size_t write(const uint8_t *buffer, size_t size) override;
    };
#ifdef BOARD_DINGTIAN
    #include <ESPTelnet.h>
    class TelnetPrint:public Print, public ESPTelnetBase {
        public:
            size_t write(uint8_t) override;
            void handleInput() override;
    };
    extern TelnetPrint debugOut;
#endif
    #define dbg(x...) debugOut.print(x);
    #define dbgln(x...) debugOut.println(x);
#endif