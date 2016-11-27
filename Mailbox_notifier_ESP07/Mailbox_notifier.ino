/*
  Inspired by: http://www.esp8266.com/viewtopic.php?f=11&t=4458&hilit=sleep+and+wake+on+gpio2+low 
  And: http://www.esp8266.com/viewtopic.php?f=11&t=4458
  Flashing with: https://labs.hybris.com/2015/07/03/esp8266/

  VCC -> VCC, GND -> GND, GPIO15 -> GND
  10K resistor between GND and CH_PD to keep it powered off until button press
  1K resistor between holdPin (GPIO2) and CH_PD to keep alive after button press 
  Button between VCC and CH_PD activates the program
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.1.5";
const char* stateTopic = "espPostkasse/reed";
const char* myHostname = "espPostkasse"; // set hostname for ESP
int holdPin = 4; // defines GPIO 4 as the hold pin (will hold CH_PD high untill we power down).

// NETWORK: Static IP details... - Should connect faster
IPAddress ip(192, 168, 1, 55);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  pinMode(holdPin, OUTPUT);  // sets GPIO 4 to output
  digitalWrite(holdPin, HIGH);  // sets GPIO 4 to high (this holds CH_PD high even if the button output goes low)
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  WiFi.config(ip, gateway, subnet);
  WiFi.hostname(myHostname); // set hostname for ESP
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      // Once connected, publish an announcement...
      client.publish(stateTopic, "1", true); // true = retain
      // digitalWrite(holdPin, LOW);  // set GPIO 4 low this takes CH_PD & powers down the ESP
      ESP.deepSleep(0); // Which is best? This one, or the one above?
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
