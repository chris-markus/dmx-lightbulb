//static const char * HEADER_KEYS[] = { "User-Agent", };

#define COLOR_BACKGROUND "#FFFFFF"
#define COLOR_FOREGROUND "#212121"
#define COLOR_LIGHT_FOREGROUND "#555555"
#define COLOR_INPUT "#FFFFFF"
#define COLOR_TEXT "#212121"
#define COLOR_TEXT_INVERT "#FFFFFF"

const char HTTP_HEADER[] PROGMEM =
  "<!DOCTYPE html><html lang=\"en\" class=\"\">"
  "<head>"
  "<meta charset='utf-8'>"
  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"/>"
  "<title>Light Controller</title>"
  "</head>";

const char HTTP_HEAD_1[] PROGMEM =
  "<style>"
  "div,fieldset,input,select{padding:5px;font-size:1em;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;}"
  "fieldset{background: " COLOR_BACKGROUND ";}"
  "p{margin:0.5em 0;}"
  "input{width:100%%;background: " COLOR_INPUT ";color: " COLOR_TEXT ";}"
  "input[type=checkbox],input[type=radio]{width:1em;margin-right:6px;vertical-align:-1px;}"
  "input[type=range]{width:99%%;}"
  "select{width:100%%;background: " COLOR_INPUT ";color: " COLOR_TEXT ";}"  
  "textarea{resize:vertical;width:98%%;height:318px;padding:5px;overflow:auto;background: " COLOR_INPUT ";color: " COLOR_TEXT ";}"
  "body{text-align:center;font-family:verdana,sans-serif;background: " COLOR_BACKGROUND ";}"
  "td{padding:0px;}"
  ".button{width: 100%; display: block; text-decoration: none; background: " COLOR_FOREGROUND "; border-radius: 5px; color: " COLOR_TEXT_INVERT"; padding: 8px; text-align: center;}"
  "</style>"
  "</head>";

const char HTTP_BODY_1[] PROGMEM =
  "<body>"
  "<div style='text-align:left;display:inline-block;color:#%06x;min-width:340px;'>";

const char HTTP_END[] PROGMEM =
  "<div style='text-align:right;font-size:11px;border-top: 1px solid " COLOR_LIGHT_FOREGROUND "'>Created by <a href='#' target='_blank' style='color:#aaa;'>Chris Markus</a></div>"
  "</div>"
  "</body>"
  "</html>";

const char HTTP_STATUS_READOUT[] PROGMEM = 
  "<div>"
  "<p>Alias: %s</p>"
  "<p>IP Address: %s</p>"
  "<p>Universe: %d | Address: %d"
  "</div>";

const char HTTP_BUTTON[] PROGMEM = 
  "<a class=\"button\" href=\"%s\">"
  "%s"
  "</a>";