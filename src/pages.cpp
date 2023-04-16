#include "pages.h"

#define ETAG "\"" __DATE__ "" __TIME__ "\""

void setupPages(AsyncWebServer *server, PhaseSwitch *phaseSwitch, Config *config, WiFiManager *wm){
  server->on("/", HTTP_GET, [phaseSwitch](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, phaseSwitch->getState().c_str());
    if (phaseSwitch->canSwitchTo1P()){
      sendPostButton(response, "Switch to 1P", "1p");
    }
    if (phaseSwitch->canSwitchTo3P()) {
      sendPostButton(response, "Switch to 3P", "3p");
    }
    sendButton(response, "Status", "status");
    sendButton(response, "Config", "config");
    sendButton(response, "Debug", "debug");
    sendButton(response, "Firmware update", "update");
    sendButton(response, "WiFi reset", "wifi", "r");
    sendButton(response, "Reboot", "reboot", "r");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/status", HTTP_GET, [phaseSwitch](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /status");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Status");
    response->print("<table>");

    // show ESP infos...
    sendTableRow(response, "ESP SSID", WiFi.SSID());
    sendTableRow(response, "ESP RSSI", (uint16_t)WiFi.RSSI());
    sendTableRow(response, "ESP WiFi Quality", WiFiQuality(WiFi.RSSI()));
    sendTableRow(response, "ESP MAC", WiFi.macAddress());
    sendTableRow(response, "ESP IP",  WiFi.localIP().toString() );
    response->print("<tr><td>&nbsp;</td><td></td></tr>");

    sendTableRow(response, "RTU Messages", phaseSwitch->getRtuMessageCount());
    sendTableRow(response, "RTU Pending Messages", phaseSwitch->getRtuPendingRequestCount());
    sendTableRow(response, "RTU Errors", phaseSwitch->getRtuErrorCount());
    sendTableRow(response, "Bridge Message", phaseSwitch->getBridgeMessageCount());
    sendTableRow(response, "Bridge Clients", phaseSwitch->getBridgeActiveClientCount());
    sendTableRow(response, "Bridge Errors", phaseSwitch->getBridgeErrorCount());
    response->print("<tr><td>&nbsp;</td><td></td></tr>");

    sendTableRow(response, "Modbus Register-Layouts Version", "0x" + String(phaseSwitch->getInputRegister(4), 16));
    sendTableRow(response, "Charging State", ChargingState(phaseSwitch->getInputRegister(5)));
    sendTableRow(response, "L1 - Current (A)", phaseSwitch->getInputRegister(6) * 0.1f);
    sendTableRow(response, "L2 - Current (A)", phaseSwitch->getInputRegister(7) * 0.1f);
    sendTableRow(response, "L3 - Curren (A)", phaseSwitch->getInputRegister(8) * 0.1f);
    sendTableRow(response, "PCB-Temperatur (°C)", phaseSwitch->getInputRegister(9) * 0.1f);
    sendTableRow(response, "Voltage L1 (V)", phaseSwitch->getInputRegister(10));
    sendTableRow(response, "Voltage L2 (V)", phaseSwitch->getInputRegister(11));
    sendTableRow(response, "Voltage L3 (V)", phaseSwitch->getInputRegister(12));
    sendTableRow(response, "extern lock state", phaseSwitch->getInputRegister(13)==0?"locked":"unlocked");
    sendTableRow(response, "Power (VA)", phaseSwitch->getInputRegister(14));
    sendTableRow(response, "Energy since PowerOn (Wh)", (uint32_t)(phaseSwitch->getInputRegister(15) << 4 | phaseSwitch->getInputRegister(16)));
    sendTableRow(response, "Energy since Installation (Wh)",  (uint32_t)(phaseSwitch->getInputRegister(17) << 4 | phaseSwitch->getInputRegister(18)));
    response->print("<tr><td>&nbsp;</td><td></td></tr>");

    sendTableRow(response, "ModBus-Master WatchDog Timeout (ms)", phaseSwitch->getHoldingRegister(257));
    sendTableRow(response, "Standby Function Control", phaseSwitch->getHoldingRegister(258)==0?"enabled":"disabled");
    sendTableRow(response, "Remote lock", phaseSwitch->getHoldingRegister(259)==0?"locked":"unlocked");
    sendTableRow(response, "Maximal current command (A)", phaseSwitch->getHoldingRegister(261) * 0.1f);
    sendTableRow(response, "FailSafe Current configuration (A)", phaseSwitch->getHoldingRegister(262) * 0.1f);
    
    response->print("<tr><td>&nbsp;</td><td></td></tr>");
    sendTableRow(response, "Build time", __DATE__ " " __TIME__);
    sendTableRow(response, "Uptime", Uptime());
    response->print("</table><p></p>");
    response->print("<form method=\"post\">"
      "<button class=\"r\">Update register</button>"
      "</form>"
      "<p></p>");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/status", HTTP_POST, [phaseSwitch](AsyncWebServerRequest *request){
    phaseSwitch->updateCachedRegisters();
    request->redirect("/status");
  });
  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /reboot");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Really?");
    sendButton(response, "Back", "/");
    response->print("<form method=\"post\">"
        "<button class=\"r\">Yes, do it!</button>"
      "</form>");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /reboot");
    request->redirect("/");
    dbgln("[webserver] rebooting...")
    ESP.restart();
    dbgln("[webserver] rebooted...")
  });
  server->on("/config", HTTP_GET, [config](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /config");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Config");
    response->print("<form method=\"post\">");
    response->print("<table>"
      "<tr>"
        "<td>"
          "<label for=\"sd\">Phase switch delay (ms)</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"1\" id=\"sd\" name=\"sd\" value=\"%d\">", config->getSwitchDelay());
    response->print("</td>"
        "</tr>"
        "</table>");
    response->print("<button class=\"r\">Save</button>"
      "</form>"
      "<p></p>");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/config", HTTP_POST, [config, phaseSwitch](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /config");
    if (request->hasParam("sd", true)){
      auto delay = request->getParam("sd", true)->value().toInt();
      config->setSwitchDelay(delay);
      phaseSwitch->setSwitchDelay(delay);
      dbgln("[webserver] saved switch delay");
    }
    request->redirect("/");
  });
  server->on("/1p", HTTP_POST, [phaseSwitch](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /1p");
    phaseSwitch->switchTo1P();
    request->redirect("/");
  });
  server->on("/3p", HTTP_POST, [phaseSwitch](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /3p");
    phaseSwitch->switchTo3P();
    request->redirect("/");
  });
  server->on("/debug", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /debug");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Debug");
    sendDebugForm(response, "1", "1", "3", "1");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/debug", HTTP_POST, [phaseSwitch](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /debug");
    String slaveId = "1";
    if (request->hasParam("slave", true)){
      slaveId = request->getParam("slave", true)->value();
    }
    String reg = "1";
    if (request->hasParam("reg", true)){
      reg = request->getParam("reg", true)->value();
    }
    String func = "3";
    if (request->hasParam("func", true)){
      func = request->getParam("func", true)->value();
    }
    String count = "1";
    if (request->hasParam("count", true)){
      count = request->getParam("count", true)->value();
    }
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Debug");
    response->print("<pre>");
    auto previous = LOGDEVICE;
    auto debug = WebPrint(previous, response);
    LOGDEVICE = &debug;
    ModbusMessage answer = phaseSwitch->sendRtuRequest(slaveId.toInt(), func.toInt(), reg.toInt(), count.toInt());
    LOGDEVICE = previous;
    response->print("</pre>");
    auto error = answer.getError();
    if (error == SUCCESS){
      auto count = answer[2];
      response->print("<span >Answer: 0x");
      for (size_t i = 0; i < count; i++)
      {
        response->printf("%02x", answer[i + 3]);
      }      
      response->print("</span>");
    }
    else{
      response->printf("<span class=\"e\">Error: %#02x (%s)</span>", error, ErrorName(error).c_str());
    }
    sendDebugForm(response, slaveId, reg, func, count);
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /update");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Firmware Update");
    response->print("<form method=\"post\" enctype=\"multipart/form-data\">"
      "<input type=\"file\" name=\"file\" id=\"file\" required/>"
      "<p></p>"
      "<button class=\"r\">Upload</button>"
      "</form>"
      "<p></p>");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    dbgln("[webserver] OTA finished");
    if (Update.hasError()){
      auto *response = request->beginResponse(500, "text/plain", "Ota failed");
      response->addHeader("Connection", "close");
      request->send(response);
    }
    else{
      request->redirect("/");
    }
    ESP.restart();
  }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    dbg("[webserver] OTA progress ");dbgln(index);
    if (!index) {
      //TODO add MD5 Checksum and Update.setMD5
      int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
      if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
        Update.printError(Serial);
        return request->send(400, "text/plain", "OTA could not begin");
      }
    }
    // Write chunked data to the free sketch space
    if(len){
      if (Update.write(data, len) != len) {
        return request->send(400, "text/plain", "OTA could not write data");
      }
    }
    if (final) { // if the final flag is set then this is the last frame of data
      if (!Update.end(true)) { //true to set the size to the current progress
        Update.printError(Serial);
        return request->send(400, "text/plain", "Could not end OTA");
      }
    }else{
      return;
    }
  });
  server->on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /wifi");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "WiFi reset");
    response->print("<p class=\"e\">"
        "This will delete the stored WiFi config<br/>"
        "and restart the ESP in AP mode.<br/> Are you sure?"
      "</p>");
    sendButton(response, "Back", "/");
    response->print("<p></p>"
      "<form method=\"post\">"
        "<button class=\"r\">Yes, do it!</button>"
      "</form>");    
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/wifi", HTTP_POST, [wm](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /wifi");
    request->redirect("/");
    wm->erase();
    dbgln("[webserver] erased wifi config");
    dbgln("[webserver] rebooting...");
    ESP.restart();
    dbgln("[webserver] rebooted...");
  });
  server->on("/favicon.ico", [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /favicon.ico");
    request->send(204);//TODO add favicon
  });
  server->on("/style.css", [](AsyncWebServerRequest *request){
    if (request->hasHeader("If-None-Match")){
      auto header = request->getHeader("If-None-Match");
      if (header->value() == String(ETAG)){
        request->send(304);
        return;
      }
    }
    dbgln("[webserver] GET /style.css");
    auto *response = request->beginResponse(200, "text/css",
    "body{"    
      "font-family:sans-serif;"
	    "text-align: center;"
      "background: #252525;"
	    "color: #faffff;"
    "}"
    "#content{"
	    "display: inline-block;"
	    "min-width: 340px;"
    "}"
    "button{"
	    "width: 100%;"
	    "line-height: 2.4rem;"
	    "background: #1fa3ec;"
	    "border: 0;"
	    "border-radius: 0.3rem;"
	    "font-size: 1.2rem;"
      "-webkit-transition-duration: 0.4s;"
      "transition-duration: 0.4s;"
	    "color: #faffff;"
    "}"
    "button:hover{"
	    "background: #0e70a4;"
    "}"
    "button.r{"
	    "background: #d43535;"
    "}"
    "button.r:hover{"
	    "background: #931f1f;"
    "}"
    "table{"
      "text-align:left;"
      "width:100%;"
    "}"
    "input{"
      "width:100%;"
    "}"
    ".e{"
      "color:red;"
    "}"
    "pre{"
      "text-align:left;"
    "}"
    );
    response->addHeader("ETag", ETAG);
    request->send(response);
  });
  server->onNotFound([](AsyncWebServerRequest *request){
    dbg("[webserver] request to ");dbg(request->url());dbgln(" not found");
    request->send(404, "text/plain", "404");
  });
}

void sendResponseHeader(AsyncResponseStream *response, const char *title){
    response->print("<!DOCTYPE html>"
      "<html lang=\"en\" class=\"\">"
      "<head>"
      "<meta charset='utf-8'>"
      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"/>");
    response->printf("<title>Heidelberg Phase Switch - %s</title>", title);
    response->print("<link rel=\"stylesheet\" href=\"style.css\">"
      "</head>"
      "<body>"
      "<h2>Heidelberg Phase Switch</h2>");
    response->printf("<h3>%s</h3>", title);
    response->print("<div id=\"content\">");
}

void sendResponseTrailer(AsyncResponseStream *response){
    response->print("</div></body></html>");
}

void sendButton(AsyncResponseStream *response, const char *title, const char *action, const char *css){
    response->printf(
      "<form method=\"get\" action=\"%s\">"
        "<button class=\"%s\">%s</button>"
      "</form>"
      "<p></p>", action, css, title);
}

void sendPostButton(AsyncResponseStream *response, const char *title, const char *action){
    response->printf(
      "<form method=\"post\" action=\"%s\">"
        "<button type=\"submit\">%s</button>"
      "</form>"
      "<p></p>", action, title);
}

void sendTableRow(AsyncResponseStream *response, const char *name, String value){
  sendTableRow(response, name, value.c_str());
}

void sendTableRow(AsyncResponseStream *response, const char *name, const char *value){
    response->printf(
      "<tr>"
        "<td>%s:</td>"
        "<td>%s</td>"
      "</tr>", name, value);
}

void sendTableRow(AsyncResponseStream *response, const char *name, float value){
  response->printf(
    "<tr>"
      "<td>%s:</td>"
      "<td>%.1f</td>"
    "</tr>", name, value);
}

void sendTableRow(AsyncResponseStream *response, const char *name, uint16_t value){
  sendTableRow(response, name, (uint32_t)value);
}

void sendTableRow(AsyncResponseStream *response, const char *name, uint32_t value){
    response->printf(
      "<tr>"
        "<td>%s:</td>"
        "<td>%d</td>"
      "</tr>", name, value);
}

void sendDebugForm(AsyncResponseStream *response, String slaveId, String reg, String function, String count){
    response->print("<form method=\"post\">");
    response->print("<table>"
      "<tr>"
        "<td>"
          "<label for=\"slave\">Slave ID</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"247\" id=\"slave\" name=\"slave\" value=\"%s\">", slaveId.c_str());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"func\">Function</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"func\" name=\"func\" data-value=\"%s\">", function.c_str());
    response->print("<option value=\"1\">01 Read Coils</option>"
              "<option value=\"2\">02 Read Discrete Inputs</option>"
              "<option value=\"3\">03 Read Holding Register</option>"
              "<option value=\"4\">04 Read Input Register</option>"
            "</select>"
          "</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"reg\">Register</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"65535\" id=\"reg\" name=\"reg\" value=\"%s\">", reg.c_str());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"count\">Count</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"65535\" id=\"count\" name=\"count\" value=\"%s\">", count.c_str());
    response->print("</td>"
        "</tr>"
      "</table>");
    response->print("<button class=\"r\">Send</button>"
      "</form>"
      "<p></p>");
    response->print("<script>"
      "(function(){"
        "var s = document.querySelectorAll('select[data-value]');"
        "for(d of s){"
          "d.querySelector(`option[value='${d.dataset.value}']`).selected=true"
      "}})();"
      "</script>");
}

const String ErrorName(Modbus::Error code)
{
    switch (code)
    {
        case Modbus::Error::SUCCESS: return "Success";
        case Modbus::Error::ILLEGAL_FUNCTION: return "Illegal function";
        case Modbus::Error::ILLEGAL_DATA_ADDRESS: return "Illegal data address";
        case Modbus::Error::ILLEGAL_DATA_VALUE: return "Illegal data value";
        case Modbus::Error::SERVER_DEVICE_FAILURE: return "Server device failure";
        case Modbus::Error::ACKNOWLEDGE: return "Acknowledge";
        case Modbus::Error::SERVER_DEVICE_BUSY: return "Server device busy";
        case Modbus::Error::NEGATIVE_ACKNOWLEDGE: return "Negative acknowledge";
        case Modbus::Error::MEMORY_PARITY_ERROR: return "Memory parity error";
        case Modbus::Error::GATEWAY_PATH_UNAVAIL: return "Gateway path unavailable";
        case Modbus::Error::GATEWAY_TARGET_NO_RESP: return "Gateway target no response";
        case Modbus::Error::TIMEOUT: return "Timeout";
        case Modbus::Error::INVALID_SERVER: return "Invalid server";
        case Modbus::Error::CRC_ERROR: return "CRC error";
        case Modbus::Error::FC_MISMATCH: return "Function code mismatch";
        case Modbus::Error::SERVER_ID_MISMATCH: return "Server id mismatch";
        case Modbus::Error::PACKET_LENGTH_ERROR: return "Packet length error";
        case Modbus::Error::PARAMETER_COUNT_ERROR: return "Parameter count error";
        case Modbus::Error::PARAMETER_LIMIT_ERROR: return "Parameter limit error";
        case Modbus::Error::REQUEST_QUEUE_FULL: return "Request queue full";
        case Modbus::Error::ILLEGAL_IP_OR_PORT: return "Illegal ip or port";
        case Modbus::Error::IP_CONNECTION_FAILED: return "IP connection failed";
        case Modbus::Error::TCP_HEAD_MISMATCH: return "TCP header mismatch";
        case Modbus::Error::EMPTY_MESSAGE: return "Empty message";
        case Modbus::Error::ASCII_FRAME_ERR: return "ASCII frame error";
        case Modbus::Error::ASCII_CRC_ERR: return "ASCII crc error";
        case Modbus::Error::ASCII_INVALID_CHAR: return "ASCII invalid character";
        default: return "undefined error";
    }
}

// translate RSSI to quality string
const String WiFiQuality(int rssiValue)
{
    switch (rssiValue)
    {
        case -30 ... 0: return "Amazing"; 
        case -67 ... -31: return "Very Good"; 
        case -70 ... -68: return "Okay"; 
        case -80 ... -71: return "Not Good"; 
        default: return "Unusable";
    }
}

const String ChargingState(uint16_t state){
  switch(state){
    case 2: return "A1";
    case 3: return "A2";
    case 4: return "B1";
    case 5: return "B2";
    case 6: return "C1";
    case 7: return "C2";
    case 8: return "derating";
    case 9: return "E";
    case 10: return "F";
    case 11: return "ERR";
      default: return String(state);
  }
}

const String Uptime(){
  char buffer[9];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", uptime::getHours(), uptime::getMinutes(), uptime::getSeconds());
  auto result = String(buffer);
  if (uptime::getDays() > 0){
    result = String(uptime::getDays()) + "." + result;
  }
  return result;
}