/*
http://www.irobot.com/filelibrary/pdfs/hrd/create/Create%20Open%20Interface_v2.pdf
https://github.com/incmve/roomba-eps8266/wiki
http://www.robotikasklubs.lv/read_write/file/Piemers/iRobot_Roomba_500_Open_Interface_Spec.pdf
http://www.airspayce.com/mikem/arduino/Roomba/index.html
*/
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
const char* ssid = "";  // your routers SSID
const char* password = "";  //your routers PASSWORD
const char* myHostname = "espRoombot"; // set hostname for ESP
int holdPin = 0; // defines GPIO 0 as the hold pin (will hold CH_PD high untill we power down).
WiFiClient client;  // starts a WiFi client.
ESP8266WebServer server ( 80 ); // starts a server on port 80


void setup() {
  WiFi.hostname(myHostname); // set hostname for ESP
  WiFi.begin(ssid, password);  // starts WiFi & login

  while (WiFi.status() != WL_CONNECTED) {  // loops untill WiFi is connected
    delay(500);
  }
  Serial.begin(115200);
  server.on ( "/", handleRoot );
  server.on ( "/clean", handleClean );
  server.on ( "/dock", handleDock );
  server.on ( "/reset", handleReset );

  server.begin();
}


void loop() {
  server.handleClient();
} // end of void loop, returns to start loop again at void loop


void handleRoot() {
  char temp[1400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 1400,

             "<!DOCTYPE html>\
<html>\
<head>\
<meta charset=\"UTF-8\">\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=1\" />\
<meta name=\"mobile-web-app-capable\" content=\"yes\" />\
<meta name=\"apple-mobile-web-app-capable\" content=\"yes\" />\
<meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black-translucent\" />\
    <title>Roombot 1.0</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from Roombot!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <input type='button' onclick=\"location.href='/clean';\" value='Clean' />\
    <input type='button' onclick=\"location.href='/dock';\" value='Dock' />\
    <input type='button' onclick=\"location.href='/reset';\" value='Restart ESP' />\
  </body>\
</html>",

             hr, min % 60, sec % 60
           );
  server.send ( 200, "text/html", temp );
}


void handleClean() {
  Serial.write(128);  // START
  delay(50);
  // Serial.write(130);  // SAFE MODE
  // delay(50);
  Serial.write(135);  // CLEAN CLEAN
  server.send ( 200, "text/plain", "I will clean master" );
}


void handleDock() {
  Serial.write(128);  // START
  delay(50);
  // Serial.write(130);  // SAFE MODE
  // delay(50);
  Serial.write(143);  // DOCK MODE
  server.send ( 200, "text/plain", "Thank you for letting me rest, going home master" );
}


void handleReset() {
  server.send ( 200, "text/html", "Restarting ESP now");
  delay(500);
  ESP.restart();
}
