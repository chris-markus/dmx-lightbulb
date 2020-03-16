// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

#include "command.h"

#define COLOR_BACKGROUND "#FFFFFF"
#define COLOR_FOREGROUND "#212121"
#define COLOR_LIGHT_FOREGROUND "#555555"
#define COLOR_INPUT "#FFFFFF"
#define COLOR_TEXT "#212121"
#define COLOR_TEXT_INVERT "#FFFFFF"

const char WEBPAGE_GLOBAL_STYLE[] PROGMEM =
  "div,a,fieldset,input,select{font-size:1em;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;}"
  "fieldset{background: " COLOR_BACKGROUND ";}"
  "p{margin:0.5em 0;}"
  "input{width:100%%;background: " COLOR_INPUT ";color: " COLOR_TEXT ";}"
  "input[type=checkbox],input[type=radio]{width:1em;margin-right:6px;vertical-align:-1px;}"
  "input[type=range]{width:99%%;}"
  "select{width:100%%;background: " COLOR_INPUT ";color: " COLOR_TEXT ";}"  
  "textarea{resize:vertical;width:98%%;height:318px;padding:5px;overflow:auto;background: " COLOR_INPUT ";color: " COLOR_TEXT ";}"
  "body{text-align:center;font-family:verdana,sans-serif;background: " COLOR_BACKGROUND ";}"
  ".button, .button-inv{width: 100%; display: block; text-decoration: none; border-radius: 5px; padding: 8px; margin: 8px 0; text-align: center; cursor: pointer}"
  ".button{background: " COLOR_FOREGROUND "; color: " COLOR_TEXT_INVERT";}"
  ".button-inv{background: " COLOR_BACKGROUND "; color: " COLOR_LIGHT_FOREGROUND"; border: 1px solid " COLOR_LIGHT_FOREGROUND "}"
  "td:nth-child(1){text-align: right;}"
  "td{padding:0px;}"
  "a{color: #aaa;}";

const char WEBPAGE_HEAD[] PROGMEM =
  "<!DOCTYPE html><html lang=\"en\" class=\"\">"
  "<head>"
  "<meta charset='utf-8'>"
  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"/>"
  "<title>Light Controller</title>"
  "<style>"
  "%s"
  "</style>"
  "</head>";

const char WEBPAGE_BODY_START[] PROGMEM =
  "<body>"
  "<div style='text-align:left;display:inline-block;color:" COLOR_TEXT ";min-width:340px;'>";

const char WEBPAGE_END[] PROGMEM =
      "<div style='text-align:right;font-size:11px;border-top: 1px solid " COLOR_LIGHT_FOREGROUND "'>"
        "<a class='button-inv' href='/'>Home</a>"
        "Created by <a href='https://github.com/chris-markus/' target='_blank'>Chris Markus</a>. <a href='https://github.com/chris-markus/dmx-lightbulb'>Source</a>"
      "</div>"
      "</div>"
    "</body>"
  "</html>";

const char WEBPAGE_STATUS_READOUT[] PROGMEM = 
  "<div>"
    "<p>Alias: %s</p>"
    "<p>IP Address: %s</p>"
    "<p>Universe: %d | Address: %d"
  "</div>";

const char WEBPAGE_BUTTON[] PROGMEM = 
  "<a class='button' href='%s'>%s</a>";

const char WEBPAGE_REBOOT_BUTTON[] PROGMEM = 
  "<script> function r() {var x = new XMLHttpRequest(); x.open('GET', '/cmd?" CMD_REBOOT_NAME "=1', true);x.send();document.body.innerHTML='<h2>Rebooting...</h2>'; window.location.href='/';}</script>"
  "<a class='button' href='#' onclick='r()' id='rebootBtn'>Reboot</a>";

// ------------------------------ Update Page --------------------------------
static const char WEBPAGE_UPDATE[] PROGMEM =
  "<h2>Arduino OTA</h2>"
  "<form method='POST' action='/cmd'>"
    "<label for='OTA'>Arduino OTA: </label>"
    "<select name='" CMD_ARDUINOOTA_NAME "' id='OTA'>"
      "<option value='" CMD_ARDUINOOTA_VALUE_TRUE "' %s >Enable</option>"
      "<option value='" CMD_ARDUINOOTA_VALUE_FALSE "' %s >Disable</option>"
    "</select>"
    "<input hidden name='" REDIRECT "' value='/update'>"
    "<input class='button' type='submit' value='Save'>"
  "</form>"
  "<h2>Firmware Upload</h2>"
  "<form method='POST' action='' enctype='multipart/form-data'>"
    "<input type='file' accept='.bin,.bin.gz' name='firmware'>"
    "<input class='button' type='submit' value='Update Firmware'>"
  "</form>"
  #ifdef ALLOW_FS_UPDATES
  "<h2>FileSystem Upload</h2>"
  "<form method='POST' action='' enctype='multipart/form-data'>"
      "<input type='file' accept='.bin,.bin.gz' name='filesystem'>"
      "<input class='button' type='submit' value='Update FileSystem'>"
  "</form>"
  #endif
  ;

static const char WEBPAGE_UPDATE_SUCCESS[] PROGMEM = 
  "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...";


// ------------------------------ Settings Page --------------------------------

// Params:
//  int dmxAddress
//  int dmxUniverse
static const char WEBPAGE_SETTINGS[] PROGMEM =
  "<form method='POST' action='/cmd'>"
    "<h2>General</h2>"
    "<h2>Network Control (ArtNet, sACN)</h2>"
    "<table>"
    "<tr>"
    "<td><label for='networkControl'>Control Protocol: </label></td>"
    "<td><select name='" CMD_NETWORK_NAME "' id='networkControl'>"
      "<option value='" CMD_NETWORK_VALUE_ARTNET "' %s >ArtNet</option>"
      "<option value='" CMD_NETWORK_VALUE_SACN "' %s >sACN</option>"
      "<option value='" CMD_NETWORK_VALUE_NONE "' %s >Disabled</option>"
    "</select></td>"
    "</tr><tr>"
    "<td><label for='address'>Address: </label></td>"
    "<td><input type='text' name='" CMD_DMXADDRESS_NAME "' placeholder='%d' id='address'></td>"
    "</tr><tr>"
    "<td><label for='universe'>Universe: </label></td>"
    "<td><input type='text' name='" CMD_DMXUNIVERSE_NAME "' placeholder='%d' id='universe'></td>"
    "</tr></table>"
    "<input hidden name='" REDIRECT "' value='/settings'>"
    "<input class='button' type='submit' value='Save'>"
  "</form>"
  ;

#endif
