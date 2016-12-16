// https://community.home-assistant.io/t/sonoff-homeassistant-alternative-firmware-for-sonoff-switches-for-use-with-mqtt-ha/2332/134
#include <MQTT.h>
#include <PubSubClient.h>
#include <PubSubClient_JSON.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

const char* myHostname = "espPanthella"; // set hostname for ESP
#define BUTTON             0                                 // (Don't Change for Sonoff)
#define RELAY             12                                 // (Don't Change for Sonoff)
#define LED               13                                 // (Don't Change for Sonoff)
#define light_switch_pin  14                                 // Button on Panthella will toggle the state

#define MQTT_CLIENT     "Sonoff_Panthella_v1.0p"             // mqtt client_id (Must be unique for each Sonoff)
#define MQTT_SERVER     "192.168.1.5"                        // mqtt server
#define MQTT_PORT       1883                                 // mqtt port
#define MQTT_TOPIC      "home/sonoff/panthella"              // mqtt topic (Must be unique for each Sonoff)
//#define MQTT_USER       ""                                 // mqtt user
//#define MQTT_PASS       ""                                 // mqtt password

#define WIFI_SSID       ""                               // wifi ssid
#define WIFI_PASS       ""                        // wifi password

#define VERSION    "\n\n------------------  Sonoff Powerpoint v1.0p  -------------------"

extern "C" {
#include "user_interface.h"
}

bool sendStatus = false;
bool requestRestart = false;

int kUpdFreq = 1;
int kRetries = 20; // Change from 10 to 20 if there are any problems..

unsigned long TTasks;
unsigned long count = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient, MQTT_SERVER, MQTT_PORT);
Ticker btn_timer;


int light_switch_State;
int last_light_switch_State;


void callback(const MQTT::Publish& pub) {
  if (pub.payload_string() == "stat") {
  }
  else if (pub.payload_string() == "on") {
    digitalWrite(LED, LOW);
    digitalWrite(RELAY, HIGH);
  }
  else if (pub.payload_string() == "off") {
    digitalWrite(LED, HIGH);
    digitalWrite(RELAY, LOW);
  }
  else if (pub.payload_string() == "reset") {
    requestRestart = true;
  }
  sendStatus = true;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(light_switch_pin, INPUT);

  digitalWrite(LED, HIGH);  //off = high
  digitalWrite(RELAY, LOW); //off = low

  btn_timer.attach(0.05, button);

  mqttClient.set_callback(callback);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(myHostname); // set hostname for ESP
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.begin(115200);
  Serial.println(VERSION);
  Serial.print("\nESP ChipID: ");
  Serial.print(ESP.getChipId(), HEX);
  Serial.print("\nConnecting to "); Serial.print(WIFI_SSID); Serial.print(" Wifi");
  while ((WiFi.status() != WL_CONNECTED) && kRetries --) {
    delay(500);
    Serial.print(" .");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" DONE");
    Serial.print("IP Address is: "); Serial.println(WiFi.localIP());
    Serial.print("Connecting to "); Serial.print(MQTT_SERVER); Serial.print(" Broker . .");
    delay(500);
//    while (!mqttClient.connect(MQTT::Connect(MQTT_CLIENT).set_keepalive(90).set_auth(MQTT_USER, MQTT_PASS)) && kRetries --) {
    while (!mqttClient.connect(MQTT::Connect(MQTT_CLIENT).set_keepalive(90)) && kRetries --) {
      Serial.print(" .");
      delay(1000);
    }
    if (mqttClient.connected()) {
      Serial.println(" DONE");
      Serial.println("\n----------------------------  Logs  ----------------------------");
      Serial.println();
      mqttClient.subscribe(MQTT_TOPIC);
      blinkLED(LED, 40, 8);
      digitalWrite(LED, HIGH);
    }
    else {
      Serial.println(" FAILED!");
      Serial.println("\n----------------------------------------------------------------");
      Serial.println();
    }
  }
  else {
    Serial.println(" WiFi FAILED!");
    Serial.println("\n----------------------------------------------------------------");
    Serial.println();
  }
}

void loop() {
  mqttClient.loop();
  timedTasks();
  checkStatus();
  light_switch();
}

void blinkLED(int pin, int duration, int n) {
  for (int i = 0; i < n; i++)  {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
  }
}

void button() {
  if (!digitalRead(BUTTON)) {
    count++;
  }
  else {
    if (count > 1 && count <= 40) {
      digitalWrite(LED, !digitalRead(LED));
      digitalWrite(RELAY, !digitalRead(RELAY));
      sendStatus = true;
    }
    else if (count > 40) {
      Serial.println("\n\nSonoff Rebooting . . . . . . . . Please Wait");
      requestRestart = true;
    }
    count = 0;
  }
}

void light_switch() {
  light_switch_State = digitalRead(light_switch_pin);

  if (light_switch_State != last_light_switch_State ) {
    digitalWrite(LED, !digitalRead(LED));
    digitalWrite(RELAY, !digitalRead(RELAY));
    sendStatus = true;
  }
  last_light_switch_State = light_switch_State;  // save light switch  state for next comparison:
}

void checkConnection() {
  if (WiFi.status() == WL_CONNECTED)  {
    if (mqttClient.connected()) {
      Serial.println("mqtt broker connection . . . . . . . . . . OK");
    }
    else {
      Serial.println("mqtt broker connection . . . . . . . . . . LOST");
      requestRestart = true;
    }
  }
  else {
    Serial.println("WiFi connection . . . . . . . . . . LOST");
    requestRestart = true;
  }
}

void checkStatus() {
  if (sendStatus) {
    if (digitalRead(LED) == LOW)  {
      mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "on").set_retain().set_qos(1));
      Serial.println("Relay . . . . . . . . . . . . . . . . . . ON");
    } else {
      mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "off").set_retain().set_qos(1));
      Serial.println("Relay . . . . . . . . . . . . . . . . . . OFF");
    }
    sendStatus = false;
  }
  if (requestRestart) {
    blinkLED(LED, 400, 4);
    ESP.restart();
  }
}

void timedTasks() {
  if ((millis() > TTasks + (kUpdFreq * 60000)) || (millis() < TTasks)) {
    TTasks = millis();
    checkConnection();
  }
}
