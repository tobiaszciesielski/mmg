#include "../include/config.hpp"
#include "base64.h"

// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void sendData(const uint8_t* message, size_t messageLenght) {
    if (messageLenght > mqttClient.getBufferSize()) {
      mqttClient.publish("test", "Pocket too big!", false);
    } else {
      mqttClient.publish("test", message, messageLenght, false);
    }
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
  mqttClient.setBufferSize(1024);
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
}

// policzyć moduły przyspieszeń

void loop() {
  
  const uint16_t buffSize = mqttClient.getBufferSize() - 24; 

  unsigned long lastTimeSend = 0;
  unsigned long diff;

  uint aviableBytes = 0;

  int i = 0;

  // ======
  // buffer
  // ======
  
  const uint packagesCount = 8, packageSize = 8, frameSize = 18;  
  uint frame_index = 0;
  uint expected_frame_index = 0;
  uint package_index = 0;
  int8_t NaN = std::numeric_limits<int8_t>::max();
  
  // buffer is 3d array which means we have got many packages 
  // which every package consinsts of `packageSize` frames. 
  // Every frame has length of `frameSize`. 
  int8_t buffer[packagesCount][packageSize][frameSize]; 

  // =============
  // state machine
  // =============
  enum class States { start, nr, data, eof };
  int8_t value = 0;
  int8_t position = 0;
  States state = States::start;
  int8_t frame[frameSize];

  // Start transmission transmission of HEX-type data (0x21 is '!')
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    // Read data from serial
    aviableBytes = Serial.available();
    if (aviableBytes > 0) {
  
      // Prevent buffer from overlow and serial blocking
      if (aviableBytes >= buffSize) {
        sendData((const uint8_t*)"Buffer overflow", 16);
        for (i = 0; i < aviableBytes; i++) {
          Serial.read();
        }
      } else {

        for (i = 0; i < aviableBytes; i++) {
          value = Serial.read();
          mqttClient.publish("test", "processing!", false);

          switch (state) {
            case States::start:
              if (value == int('@'))
                state = States::nr;
              break;

            case States::nr:
              if (value >= int('1') && value <= int('8')){
                state = States::data;
                frame_index = value - int('0') - 1;
              } else {
                state = States::start;
              }
              break;

            case States::data:
              frame[position] = value;
              position++;
              if(position == 18) {
                position = 0;
                state = States::eof;
              }
              break;
            
            case States::eof:
              if (value == int('#')) {
                // if package is correct, process the frame

                // Check the correctness of incoming frame index
                if (false) { // (expected_frame_index != frame_index) {
                  
                  // if we have gap inside one package we dont switch to next package
                  if (frame_index > expected_frame_index) {
                    
                    // fill gap with NaN (max of value of data type)
                    for (int j = expected_frame_index; j <= frame_index; j++) {
                      for (int k = 0; k < frameSize; k++) {
                        buffer[package_index][j][k] = NaN;
                      }
                    }
                    
                  } else if (frame_index < expected_frame_index) {
                    int pos, steps = packageSize - abs(int(expected_frame_index - frame_index));
                    
                    // fill gap with NaN (max of value of data type)
                    for (int j = 0; j < steps; j++) {
                      pos = (expected_frame_index + j) % packageSize;
                      
                      // if we we fill package go to next one
                      if (pos == 0) {
                        package_index++;
                        
                        // buffer overflow
                        if (package_index > packagesCount-1) package_index = 0;
                      }
                      
                      for (int k = 0; k < frameSize; k++) {
                        buffer[package_index][pos][k] = NaN;
                      }
                    }

                    // if incoming frame is 0, we dont want to overwrite package filled with NaN's.
                    // we switch to new package
                    if (frame_index == 0) {
                      package_index++;
                      if (package_index > packagesCount-1) package_index = 0;
                    }
                  }
                  expected_frame_index = frame_index;
                } 

                // save data in buffer
                for(int j=0; j<frameSize; j++) {
                  buffer[package_index][frame_index][j] = frame[j]; 
                }

                // In next step we are expecting next frame (with previous index +1)
                expected_frame_index++;

                // if frame index is higher than package size, current package is full so pick next package
                if (expected_frame_index > packageSize-1) {
                  package_index++;

                  // if package index is higher than packages count than buffer overflows 
                  if (package_index > packagesCount-1){
                    package_index = 0;
                  }
                }
                
                // frame index is never higher than package size because package consists of frames
                expected_frame_index%=packageSize;

              } 
              // otherwise, ignore it and look for begining of next frame
              state = States::start;
              break;  
          
            default:
              break;
          }
        }
      }
    }

    // Send data end clear buffer
    diff = micros() - lastTimeSend;
    if (diff >= dataTimeSendMicrosec) {
      // if (package_index > 0) {
        
        char message[20];
        sprintf(message, "%u, %u", package_index, diff);
        mqttClient.publish("test", message, false);

        // sendData(buffer, bufferPosition);

        // clear buffer
        // frame_index = 0, expected_frame_index = 0, package_index = 0;
        lastTimeSend = micros();
      // }
    }
  } 
  mqttReconnect();
}