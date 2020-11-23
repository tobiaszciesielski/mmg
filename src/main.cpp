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

void sendData(const char* message, size_t messageLenghta) {
  mqttClient.publish("test", message, messageLenghta);
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.setRxBufferSize(SERIAL_BUFFER_SIZE);
  connectToWifi();
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
}

void loop() {
  
  // declare neccesary variables 
  // const int packageLength = 21;
  // unsigned long lastTimeSend = 0;

  char buffer[SERIAL_BUFFER_SIZE];

  // Start transmission transmission of HEX-type data (0x21 is !)
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    // int bytesCount = Serial.available();
    // if (bytesCount > 0) {
      
    //   // read bytes
    //   int n = Serial.readBytes(buffer, bytesCount);

    //   for (size_t i = 0; i < n;) {
        
    //     // prevent from range error 
    //     if (i + packageLength >= n) break;

    //     // check if package is correct
    //     if (buffer[i] == '@' 
    //       && (buffer[i+1] > 0 && buffer[i+1] <= 8) 
    //       &&  buffer[i+20] == '#') {

    //       std::stringstream ss;
    //       for (size_t j = 0; j < packageLength; j++) {
    //         char sample[4];
    //         sprintf(sample, "%02X:", buffer[i+j]);
    //         ss << sample;
    //       }

    //       char t[10];
    //       long timeDifference = millis() - lastTimeSend;
    //       sprintf(t, "%u", (unsigned int)timeDifference);
    //       ss << t;
    //       sendData(ss.str().c_str(), ss.str().size());
          
    //       i += packageLength;
    //       lastTimeSend = millis();
    //     }
    //     else i++;
    //   }
    // }
    

    //// READING BUFFER USING Serial.readBytes()

    // if (Serial.available() > 0) {

    //   int n = Serial.readBytes(buffer, Serial.available());

    //   std::stringstream ss;
    //   for (size_t i = 0; i < n; i++) {
    //     char sample[4];
    //     sprintf(sample, "%02X:", buffer[i]);
    //     ss << sample;
    //   }
    //   ss << n << '/'<< Serial.available() << "\n";

    //   sendData(ss.str().c_str(), ss.str().size());
    // } 


    //// READING BUFFER USING Serial.read()


    int aviableBytes = Serial.available();
    if (aviableBytes > 0) {
      std::stringstream ss;
      for (size_t i = 0; i < aviableBytes; i++) {
        char sample[4];
        sprintf(sample, "%02X:", Serial.read());
        ss << sample;
      }

      ss << aviableBytes << '/'<< Serial.available() << "\n";

      sendData(ss.str().c_str(), ss.str().size());
    }
  } 
  mqttReconnect();
}