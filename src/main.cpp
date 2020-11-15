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

void sendData(unsigned char* message, size_t messageLenghta) {
  mqttClient.publish("test", message, messageLenghta);
}

void setup() {
  Serial.begin(BAUD_RATE);
  connectToWifi();
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
}

void loop() {
  
  // declare neccesary variables 
  byte startByte = '@';
  byte endByte = '#';
  const int packageLength = 21;
  byte package[packageLength];

  // test buffer
  unsigned char buff[23];
  int j = 0;

  // Start transmission transmission of HEX-type data (0x21 is !)
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    if (Serial.available()) {
      // read bytes
      int n = Serial.readBytes(package, packageLength);
      
      // check package correctness
      if(package[0] == startByte && package[packageLength-1] == endByte) {

        // send sample
        for (size_t i = 0; i < packageLength; i++) {
          buff[i] = (unsigned char)('a' + j);
        }

        // number of sensor
        buff[21] = package[1] + '0';
        buff[22] = '\0';
        
        sendData(buff, 23);   
        j++;                              
      }
    }
  }

  mqttReconnect();
}