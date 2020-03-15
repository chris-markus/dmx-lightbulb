// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <ESP8266WebServer.h>
#include "StreamString.h"
#include <flash_hal.h>
//#include <FS.h>

#include "driver.h"
#include "settings.h"
#include "web_content.h"
#include "command.h"

int webserverPoll = 50; // ms
bool webserverEnabled = true;

// TODO: delete if remains unused
String _webContentBuffer = "";

ESP8266WebServer server(80);

void StartWebserver() {

  String updateURL = "/update";
  setupUpdatePage(updateURL);

  server.on("/", handleRoot);
  server.on("/settings", handleSettings);
  server.on("/console", handleConsole);
  server.onNotFound(handleNotFound);

  server.on("/cmd", handleCmd);

  server.begin();
  Serial.println("WebServer started");
}

void WebserverLoop() {
  server.handleClient();
}

// *ahem* borrowed from https://github.com/arendst/Tasmota
void WebserverSendHeaders()
{
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  server.sendHeader(F("Pragma"), F("no-cache"));
  server.sendHeader(F("Expires"), F("-1"));
}

void WebserverHead(String title) {
  int len = sizeof(WEBPAGE_HEAD) + sizeof(WEBPAGE_GLOBAL_STYLE);
  WebserverAddFragment(len, WEBPAGE_HEAD, WEBPAGE_GLOBAL_STYLE);
}

void WebserverStartPage(String pageTitle, int status = 200) {
  server.client().flush();
  WebserverSendHeaders();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(status, F("text/html"), "");
  WebserverHead(pageTitle);
  server.sendContent(WEBPAGE_BODY_START);
  //_webContentBuffer.clear()
}

void WebserverEndPage() {
  server.sendContent(WEBPAGE_END);
  //_webContentBuffer.clear();
}

void WebserverAddFragment(int bufferSize, const char* format, ...) {
  char* buff = new char[bufferSize];
  va_list arg;
  va_start(arg, format);
  int len = vsnprintf(buff, bufferSize, format, arg);
  va_end(arg);

  server.sendContent(buff);
  delete buff;
}

// ---------------------------- Components -------------------------------

void WebserverStatusReadout() {
  int len = sizeof(WEBPAGE_STATUS_READOUT) + 100;
  WebserverAddFragment(len, WEBPAGE_STATUS_READOUT, "Light Name", Settings.ipAddress, Settings.DMXUniverse, Settings.DMXAddress);
}

void WebserverButton(String title, String linkURL) {
  int len = sizeof(WEBPAGE_BUTTON) + title.length() + linkURL.length() + 1;
  WebserverAddFragment(len, WEBPAGE_BUTTON, linkURL.c_str(), title.c_str());
}

// --------------------------- Route Handlers ----------------------------

void handleRoot() {
  // TODO: Add the address or name here
  WebserverStartPage("DMX Lightbulb");
  WebserverStatusReadout();
  WebserverButton(F("Settings"), F("/settings"));
  WebserverButton(F("Console"), F("/console"));
  WebserverButton(F("Firmware Update"), F("/update"));
  WebserverEndPage();
}

void handleSettings() {
  const char selected[] = "selected";
  const char empty[] = "";
  WebserverStartPage("Settings");
  WebserverStatusReadout();
  WebserverAddFragment(
    sizeof(WEBPAGE_SETTINGS) + 1 + 3 + 3, 
    WEBPAGE_SETTINGS,
    Settings.artnetEnable?selected:empty,
    Settings.sACNEnable?selected:empty,
    (!Settings.artnetEnable && !Settings.sACNEnable)?selected:empty,
    Settings.DMXAddress,
    Settings.DMXUniverse);
  WebserverEndPage();
}

void handleConsole() {
  WebserverStartPage("Console");
  WebserverEndPage();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleCmd() {
  bool status = false;
  String redirectURI = "";
  String msg = "";
  if (server.args() == 0) {
    server.send(200, "text/plain", "NO COMMAND");
    return;
  }

  for (uint8_t i = 0; i < server.args(); i++) {
    for (int l=0; l<num_commands; l++) {
      if (strcmp(commands[l].cmdName, server.argName(i).c_str()) == 0) {
        // set status true if at least 1 command completes
        status |= commands[l].runCommand(commands[l].setting, server.arg(i).c_str());
        break;
      }
    }

    if (server.argName(i) == F("redirect")) {
      redirectURI = server.arg(i);
    }
  }

  if (redirectURI != "") {
    server.sendHeader("Location", redirectURI, true);
    server.send( 302, "text/plain", "");
  }
  else {
    // TODO: send back a real json response
    server.send(200, "text/plain", status?"OK":"FAIL");
  }
}

// --------------------------- Utilities ----------------------------

bool setColor(const String colorString) {
  Settings.artnetEnable = false;
  Settings.sACNEnable = false;
  if (colorString.length() >= 8) {
    int start = 0;
    if(colorString.substring(0,1) == "#") {
      start = 1;
    }
    unsigned long colorVal = strtoll(colorString.substring(start).c_str(), NULL, 16);
    Serial.println(colorString.substring(start));
    Serial.println(colorVal);
    setLEDColor((uint8_t)((colorVal>>((unsigned long)(24)))), (uint8_t)((colorVal&0xFF0000)>>16), (uint8_t)((colorVal&0xFF00)>>8), (uint8_t)(colorVal&0xFF));
    Serial.print("Setting Color: ");
    Serial.println((uint8_t)((colorVal>>24)));
    return true;
  }
  return false;
}

String _updaterError = "";
bool _serial_output = true;

void _setUpdaterError() {
  if (_serial_output) {
    Update.printError(Serial);
  }
  StreamString str;
  Update.printError(str);
  _updaterError = str.c_str();
}

void setupUpdatePage(String& path) {
  // handler for the /update form page
  server.on(path.c_str(), HTTP_GET, [&](){
    const char selected[] = "selected";
    const char empty[] = "";
    WebserverStartPage("Update Firmware");
    WebserverAddFragment(sizeof(WEBPAGE_UPDATE) + 20, 
      WEBPAGE_UPDATE,
      Settings.arduinoOTAEnable?selected:empty, 
      !Settings.arduinoOTAEnable?selected:empty);
    WebserverEndPage();
  });

  // handler for the /update form POST (once file upload finishes)
  server.on(path.c_str(), HTTP_POST, [&](){
    if (Update.hasError()) {
      server.send(200, F("text/html"), String(F("Update error: ")) + _updaterError);
    } else {
      server.client().setNoDelay(true);
      server.send_P(200, PSTR("text/html"), WEBPAGE_UPDATE_SUCCESS);
      delay(100);
      server.client().stop();
      ESP.restart();
    }
  },[&](){
    // handler for the file upload, get's the sketch bytes, and writes
    // them through the Update object
    HTTPUpload& upload = server.upload();

    if(upload.status == UPLOAD_FILE_START) {

      WiFiUDP::stopAll();
      if (_serial_output)
        Serial.printf("Update: %s\n", upload.filename.c_str());
      if (upload.name == "filesystem") {
        size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
        close_all_fs();
        if (!Update.begin(fsSize, U_FS)){//start with max available size
          if (_serial_output) Update.printError(Serial);
        }
      } else {
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace, U_FLASH)){//start with max available size
          _setUpdaterError();
        }
      }
    } else if(upload.status == UPLOAD_FILE_WRITE && !_updaterError.length()){
      if (_serial_output) Serial.printf(".");
      if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
        _setUpdaterError();
      }
    } else if(upload.status == UPLOAD_FILE_END && !_updaterError.length()){
      if(Update.end(true)){ //true to set the size to the current progress
        if (_serial_output) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        _setUpdaterError();
      }
      if (_serial_output) Serial.setDebugOutput(false);
    } else if(upload.status == UPLOAD_FILE_ABORTED){
      Update.end();
      if (_serial_output) Serial.println("Update was aborted");
    }
    delay(0);
  });
}

struct Driver drv_webServer = {
  &webserverPoll,
  StartWebserver,
  WebserverLoop,
  &webserverEnabled
};