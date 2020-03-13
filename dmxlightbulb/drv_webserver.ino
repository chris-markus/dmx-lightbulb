// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <ESP8266WebServer.h>
//#include <DNSServer.h>

#include "driver.h"
#include "settings.h"
#include "web_content.h"

int webserverPoll = 50; // ms
bool webserverEnabled = true; // ms

ESP8266WebServer server(80);

void StartWebserver() {
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.on("/cmd", handleCmd);

  server.begin();
  Serial.println("WebServer started");
}

void WebserverLoop() {
  server.handleClient();
}

// *ahem* borrowed from https://github.com/arendst/Tasmota
void WebserverSendHeader(void)
{
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  server.sendHeader(F("Pragma"), F("no-cache"));
  server.sendHeader(F("Expires"), F("-1"));
}

void handleRoot() {
  server.client().flush();
  WebserverSendHeader();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(HTTP_HEADER);
  server.sendContent(HTTP_HEAD_1);
  server.sendContent(HTTP_BODY_1);
  sendStatusReadout();
  sendButton(F("DMX Settings"), F("/settings"));
  server.sendContent(HTTP_END);
}

void sendStatusReadout() {
  int len = sizeof(HTTP_STATUS_READOUT) + 100;
  char *buff = new char[len];
  snprintf(buff, len, HTTP_STATUS_READOUT, "Light Name", Settings.ipAddress, Settings.DMXUniverse, Settings.DMXAddress);
  server.sendContent(buff);
  delete buff;
}

void sendButton(String title, String linkURL) {
  int len = sizeof(HTTP_BUTTON) + title.length() + linkURL.length() + 1;
  char *buff = new char[len];
  snprintf(buff, len, HTTP_BUTTON, linkURL.c_str(), title.c_str());
  server.sendContent(buff);
  delete buff;
}

void handleCmd() {
  bool status = true;
  if (server.args() == 0) {
    // send page to select commands
  }
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == F("color")) {
      if (!setColor(server.arg(i))) {
        status = false;
      }
    }
    else if (server.argName(i) == F("artnetEnable")) {
      if (server.arg(i) == F("1")) {
        Settings.artnetEnable = true;
      }
      else {
        Settings.artnetEnable = false;
      }
    }
    else {
      status = false;
    }
  }
  server.send(status?200:500, "text/plain", status?"OK":"FAIL");
}

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

struct Driver drv_webServer() {
  return (Driver){&webserverPoll, StartWebserver, WebserverLoop, &webserverEnabled};
}

/*
void StartWebserver(int type, IPAddress ipweb)
{
  if (!Settings.web_refresh) { Settings.web_refresh = HTTP_REFRESH_TIME; }
  if (!Web.state) {
    if (!WebServer) {
      WebServer = new ESP8266WebServer((HTTP_MANAGER == type || HTTP_MANAGER_RESET_ONLY == type) ? 80 : WEB_PORT);
      WebServer->on("/", HandleRoot);
      WebServer->onNotFound(HandleNotFound);
      WebServer->on("/up", HandleUpgradeFirmware);
      WebServer->on("/u1", HandleUpgradeFirmwareStart);  // OTA
      //XdrvCall(FUNC_WEB_ADD_HANDLER);
      //XsnsCall(FUNC_WEB_ADD_HANDLER);
    }

    WebServer->begin(); // Web server start
  }
  if (Web.state != type) {
#if LWIP_IPV6
    String ipv6_addr = WifiGetIPv6();
    if(ipv6_addr!="") AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_HTTP D_WEBSERVER_ACTIVE_ON " %s%s " D_WITH_IP_ADDRESS " %s and IPv6 global address %s "), my_hostname, (Wifi.mdns_begun) ? ".local" : "", ipweb.toString().c_str(),ipv6_addr.c_str());
    else AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_HTTP D_WEBSERVER_ACTIVE_ON " %s%s " D_WITH_IP_ADDRESS " %s"), my_hostname, (Wifi.mdns_begun) ? ".local" : "", ipweb.toString().c_str());
#else
    AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_HTTP D_WEBSERVER_ACTIVE_ON " %s%s " D_WITH_IP_ADDRESS " %s"), my_hostname, (Wifi.mdns_begun) ? ".local" : "", ipweb.toString().c_str());
#endif // LWIP_IPV6 = 1
    rules_flag.http_init = 1;
  }
  if (type) { Web.state = type; }
}

void StopWebserver(void)
{
  if (Web.state) {
    WebServer->close();
    Web.state = HTTP_OFF;
    AddLog_P(LOG_LEVEL_INFO, PSTR(D_LOG_HTTP D_WEBSERVER_STOPPED));
  }
}

void WifiManagerBegin(bool reset_only)
{
  // setup AP
  if (!global_state.wifi_down) {
//    WiFi.mode(WIFI_AP_STA);
    WifiSetMode(WIFI_AP_STA);
    AddLog_P(LOG_LEVEL_DEBUG, PSTR(D_LOG_WIFI D_WIFIMANAGER_SET_ACCESSPOINT_AND_STATION));
  } else {
//    WiFi.mode(WIFI_AP);
    WifiSetMode(WIFI_AP);
    AddLog_P(LOG_LEVEL_DEBUG, PSTR(D_LOG_WIFI D_WIFIMANAGER_SET_ACCESSPOINT));
  }

  StopWebserver();

  DnsServer = new DNSServer();

  int channel = WIFI_SOFT_AP_CHANNEL;
  if ((channel < 1) || (channel > 13)) { channel = 1; }

#ifdef ARDUINO_ESP8266_RELEASE_2_3_0
  // bool softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0);
  WiFi.softAP(my_hostname, WIFI_AP_PASSPHRASE, channel, 0);
#else
  // bool softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0, int max_connection = 4);
  WiFi.softAP(my_hostname, WIFI_AP_PASSPHRASE, channel, 0, 1);
#endif

  delay(500); // Without delay I've seen the IP address blank
  // Setup the DNS server redirecting all the domains to the apIP 
  DnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  DnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

  StartWebserver((reset_only ? HTTP_MANAGER_RESET_ONLY : HTTP_MANAGER), WiFi.softAPIP());
}

void PollDnsWebserver(void)
{
  if (DnsServer) { DnsServer->processNextRequest(); }
  if (WebServer) { WebServer->handleClient(); }
}


//********************************************************************************************

void HandleRoot(void)
{
  if (WebServer->hasArg("rst")) {
    WebRestart(0);
    return;
  }
}*/