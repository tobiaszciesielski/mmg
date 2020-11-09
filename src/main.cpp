#include "../include/config.hpp"

// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectToWifi() {
  if (DEBUG) { 
    Serial.print("Connecting to ");
    Serial.print(WIFI_NETWORK);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  
  unsigned long startAttemptTime = millis();
  
  while (WiFi.status() != WL_CONNECTED &&
          millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
    if (DEBUG) {
      Serial.print(".");
      delay(1000);
    }
  }
  
  if (DEBUG) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connection Failed !");
    } else {
      Serial.print("Connected to");
      Serial.println(WiFi.localIP());
    }
  }
}
  

void mqttReconnect() {
  while (!mqttClient.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");

    WiFi.mode(WIFI_STA);
    bool isConnected = mqttClient.connect("esp32client");
    
    if (DEBUG) {
      if (isConnected) {
        Serial.println("connected!\n"); 
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }
}


void sendData(char* message, size_t messageLenghta) {
  mqttClient.publish("test", message, messageLenghta);
}


void setup() {
  Serial.begin(BAUD_RATE);
  connectToWifi();
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
}

bool tested = false;
void loop() {
  
  // Start transmission transmission of HEX-type data (0x21 is !)
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    if (Serial.available()) {
      // read byte
      unsigned char byte = Serial.read();

      // convert byte to string
      char sample[3];
      sprintf(sample, "%x", byte);     

      // send sample
      sendData(sample, 3);                                    
    }
  }

  mqttReconnect();
}