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
  unsigned long lastTimeSend = 0;
  uint aviableBytes = 0;
  const uint16_t buffSize = mqttClient.getBufferSize() - 10;
  char buffer[buffSize];
  unsigned long diff;

  // Start transmission transmission of HEX-type data (0x21 is !)
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    // READING BUFFER USING Serial.read() with 1 milisec 
    diff = micros() - lastTimeSend;
    if(diff >= dataTimeSendMicrosec) {

      aviableBytes = Serial.available();
      if (aviableBytes > 0) {
        
        if(aviableBytes > buffSize) {
          // Prevent buffer from overlow and serial blocking
          for (size_t i = 0; i < aviableBytes; i++) {
            Serial.read();
          }
          sendData("Buffer overflow", 16);

        } else {
          // read data from buffer
          for (size_t i = 0; i < aviableBytes; i++) {
            buffer[i] = Serial.read();
          }
          sendData(buffer, aviableBytes);
        }

        lastTimeSend = micros();
      }
    }
    /*
            ^
            |
    OUTPUT OF ABOVE CODE(1kHz)
      @ûµ-
      @\übð4
      @aüÊ
      @ûµ-
      @`ü_ð5
    */

    // // READING BUFFER USING Serial.read() with 1 milisec 
    // diff = micros() - lastTimeSend;
    // if(diff >= dataTimeSendMicrosec) {
    //   aviableBytes = Serial.available();
    //   if (aviableBytes > 0) {
      
    //     for (size_t i = 0; i < aviableBytes; i++) {
    //       buffer[i] = Serial.read();
    //     }

    //     int x = sprintf(message, "%u %u", aviableBytes, diff);

    //     sendData(message, x);
    //     lastTimeSend = micros();
    //   }
    // }
    // /*
    //         ^
    //         |
    // OUTPUT OF ABOVE CODE(1kHz)
    //   42 1013
    //   126 1026
    //   21 1879
    //   105 1026
    //   42 1022
    //   42 1099
    // */


    // // Counting time between data send
    // aviableBytes = Serial.available();
    // if (aviableBytes > 0) {
     
    //   for (size_t i = 0; i < aviableBytes; i++) {
    //     buffer[i] = Serial.read();
    //   }

    //   diff = micros() - lastTimeSend;
    //   int x = sprintf(message, "%u %u", aviableBytes, diff);

    //   sendData(message, x);
    //   lastTimeSend = micros();
    // }

    // // READING BUFFER USING Serial.readBytes()
    // diff = micros() - lastTimeSend;
    // if(diff > oneSecInMicros) {
    //   Serial.print("hello \n");
    //   sendData("Hello", 6);
    //   lastTimeSend = micros();      
    // }

    // // READING BUFFER USING Serial.readBytes()
    // diff = millis() - lastTimeSend;
    // if(diff > messageSendingTime) {    
    //   aviableBytes = Serial.available();
    //   if (aviableBytes) {
    //     n = Serial.readBytes(buffer, aviableBytes);
    //     ss << n << ":" << diff << "\n";
    //     sendData(ss.str().c_str(), ss.str().size());
    //     lastTimeSend = millis();
    //     ss.clear();
    //   } 
    // }

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
    
    //// READING BUFFER USING Serial.read()
    // int aviableBytes = Serial.available();
    // if (aviableBytes > 0) {
    //   std::stringstream ss;
    //   for (size_t i = 0; i < aviableBytes; i++) {
    //     char sample[4];
    //     sprintf(sample, "%02X:", Serial.read());
    //     ss << sample;
    //   }

    //   ss << aviableBytes << '/'<< Serial.available() << "\n";

    //   sendData(ss.str().c_str(), ss.str().size());
    // }
  } 
  mqttReconnect();
}