#include "../include/config.hpp"

// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void sendData(char* message, size_t messageLenght) {
  if(messageLenght > mqttClient.getBufferSize()) {
    message="Pocket too big!", messageLenght=16;
  } 
  mqttClient.publish("test", message, messageLenght);
}

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

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.setRxBufferSize(SERIAL_BUFFER_SIZE);
  connectToWifi();
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
}

void loop() {
  
  // neccesary variables declaration
  // char bigBuffer[SERIAL_BUFFER_SIZE];
  uint bufferPosition = 0;

  const uint16_t buffSize = mqttClient.getBufferSize() - 10;
  char buffer[buffSize];

  unsigned long lastTimeSend = 0;
  unsigned long diff;
  
  uint aviableBytes = 0;

  // Start transmission transmission of HEX-type data (0x21 is !)
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    // Read data from serial
    aviableBytes = Serial.available();
    if (aviableBytes > 0) {
      if(aviableBytes > buffSize) {
        // Prevent buffer from overlow and serial blocking
        sendData("Buffer overflow", 16);
        for (size_t i = 0; i < aviableBytes; i++) {
          Serial.read();
        }
      } else {
        // Store data in buffer
        int margin = bufferPosition+aviableBytes;
        for (; bufferPosition < margin; bufferPosition++) {
          buffer[bufferPosition] = Serial.read();
        }
      }
    }

    // Send data end clear buffer
    diff = micros() - lastTimeSend;
    if(diff >= dataTimeSendMicrosec) {
      if (bufferPosition > 0) {
        // int n = sprintf(message, "%u, %u", bufferPosition, diff);
        // sendData(message, n);

        sendData(buffer, bufferPosition);
        lastTimeSend = micros();
        bufferPosition = 0;
      }
    }
  } 
  mqttReconnect();
}