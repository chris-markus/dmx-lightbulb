// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <ESP8266WebServer.h>
#include "StreamString.h"
#include <flash_hal.h>
#include <WiFiUdp.h>
//#include <FS.h>

#include "driver.h"
#include "SettingsManager.h"
#include "web_content.h"
#include "InstanceManager.h"


int webserverPoll = 50; // ms
bool webserverEnabled = true;

// TODO: delete if remains unused
String _webContentBuffer = "";

ESP8266WebServer server(80);

void handleRoot();
void setupUpdatePage(String& path);
void handleSettings();
void handleConsole();
void handleNotFound();
void handleCmd();
void WebserverAddFragment(int bufferSize, const char* format, ...);

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

void StopWebserver() {
  Serial.println("WebServer stopped");
  server.stop();
}

void WebserverLoop() {
  server.handleClient();
}

void WebserverCleanup() {

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
  // TODO: use a pool
  char* buff = (char *)malloc(bufferSize + 1);
  va_list arg;
  va_start(arg, format);
  size_t len = vsnprintf(buff, bufferSize, format, arg);
  va_end(arg);

  server.sendContent(buff, len);
  free(buff);
}

void WebserverAddFragment(const char* format) {
  server.sendContent(format);
}

// ---------------------------- Components -------------------------------

void WebserverStatusReadout() {
  char_array_16 ip {};
  strncpy(ip.data(), globals.wifi_mgr.ip_address().c_str(), 16);
  int len = sizeof(WEBPAGE_STATUS_READOUT)
            + sizeof(char_array_16)
            + Settings::alias.size()
            + ip.size()
            + Settings::DMXUniverse.size()
            + Settings::DMXAddress.size();
  WebserverAddFragment(len, WEBPAGE_STATUS_READOUT, Settings::alias.get().data(), ip.data(), Settings::DMXUniverse.get(), Settings::DMXAddress.get());
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
  WebserverAddFragment(WEBPAGE_REBOOT_BUTTON);
  WebserverEndPage();
}

void handleSettings() {
  static constexpr const char selected[] = "selected";
  static constexpr const char empty[] = "";
  WebserverStartPage("Settings");
  WebserverStatusReadout();
  const int len = sizeof(WEBPAGE_SETTINGS)
    + sizeof(selected) * 3
    + Settings::alias.size()
    + Settings::ssid.size()
    + Settings::mqtt_broker.size()
    + Settings::mqtt_username.size()
    + Settings::DMXAddress.size()
    + Settings::DMXUniverse.size();
  WebserverAddFragment(
    len,
    WEBPAGE_SETTINGS,
    Settings::alias.get().data(),
    Settings::artnetEnable.get()?selected:empty,
    Settings::sACNEnable.get()?selected:empty,
    (!Settings::artnetEnable.get() && !Settings::sACNEnable.get())?selected:empty,
    Settings::ssid.get().data(),
    Settings::mqtt_broker.get().data(),
    Settings::mqtt_username.get().data(),
    Settings::DMXAddress.get(),
    Settings::DMXUniverse.get());
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
    for (uint8_t l=0; l<num_commands; l++) {
      if (strcmp(commands[l].cmdName, server.argName(i).c_str()) == 0) {
        // set status true if at least 1 command completes
        status |= commands[l].runCommand(commands[l].setting, server.arg(i).c_str());
        break;
      }
    }

    if (server.argName(i) == REDIRECT) {
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
      Settings::arduinoOTAEnable.get()?selected:empty, 
      !Settings::arduinoOTAEnable.get()?selected:empty);
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

static inline bool get_enabled() {
  return webserverEnabled;
}

static inline int get_poll_delay() {
  return webserverPoll;
}

struct Driver drv_webServer = {
  get_poll_delay,
  StartWebserver,
  WebserverLoop,
  WebserverCleanup,
  get_enabled
};