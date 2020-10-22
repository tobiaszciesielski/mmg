#include <Arduino.h>
  
// Additional libraries
#include "PubSubClient.h"
#include "WiFi.h"
#include "ArduinoJson.h"

// Configuration variables  
#define WIFI_NETWORK "Ciesielski Hot Spot"
#define WIFI_PASSWORD "Aneta1999"
#define WIFI_TIMEOUT_MS 15000
#define MQTT_SERVER_IP "192.168.8.120"
#define MQTT_SERVER_PORT 1883
#define SENDING_DATA_FREQ 1000
unsigned long messageSendingTime = (1.0 / SENDING_DATA_FREQ) * 1000;
  
// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);
  
void connectToWifi() {
  Serial.print("Connecting to ");
  Serial.print(WIFI_NETWORK);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  
  unsigned long startAttemptTime = millis();
  
  while (WiFi.status() != WL_CONNECTED &&
          millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
    Serial.print(".");
    delay(1000);
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connection Failed !");
  } else {
    Serial.print("Connected to");
    Serial.println(WiFi.localIP());
  }
}
  
void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    WiFi.mode(WIFI_STA);
    if (mqttClient.connect("esp32client")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendData(char* message, size_t messageLenght) {
  mqttClient.publish("test", message, messageLenght);
}

void setup() {
  Serial.begin(9600);
  connectToWifi();
  mqttClient.setBufferSize(1024);
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  Serial.print("The message sending interval is set to (ms): ");
  Serial.print(messageSendingTime);
}

unsigned long lastMessageTimeStamp = 0;
StaticJsonDocument<1024> jsonBuffer;
JsonArray data = jsonBuffer.createNestedArray("data");
char buffer[256];
void loop() {
  if(!mqttClient.connected())
    reconnect();

  mqttClient.loop();
  data.add(int(random(0,10)));
  
  if(millis() - lastMessageTimeStamp > messageSendingTime) {
    int n = serializeJson(jsonBuffer, buffer);
    sendData(buffer, n);

    jsonBuffer.clear();
    data = jsonBuffer.createNestedArray("data");
    jsonBuffer["interval"] = millis() - lastMessageTimeStamp;
    lastMessageTimeStamp = millis();
  }
}

