#include "../include/config.hpp"
#include <base64.h>

// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void sendData(const uint8_t* message, size_t messageLenght) {
    if (messageLenght > mqttClient.getBufferSize()) {
      mqttClient.publish("test", "Pocket too big!",false);
    } else {
      mqttClient.publish("test", message,messageLenght,false);
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
  mqttClient.setBufferSize(2000);
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
}

// policzyć moduły przyspieszeń

void loop() {
  
  // neccesary variables declaration
  uint bufferPosition = 0;
  const uint16_t buffSize = 1000; 
  uint8_t buffer[buffSize];

  int8_t counter = 0;

  unsigned long lastTimeSend = 0;
  unsigned long diff;
  char message[15];
  int margin = 0;
  uint aviableBytes = 0;
  CircularBuffer<unsigned short> circularBuffer(72, 8);

  // Start transmission transmission of HEX-type data (0x21 is !)
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    // Read data from serial
    aviableBytes = Serial.available();
    if (aviableBytes > 0) {
      margin = bufferPosition+aviableBytes;
      if(margin > buffSize) {
        // Prevent buffer from overlow and serial blocking
        sendData((const uint8_t*)"Buffer overflow", 16);
        for (size_t i = 0; i < aviableBytes; i++) {
          Serial.read();
        }
      } else {
        int8_t sample;
        bool is_frame_started = false;
        bool is_inside_frame = false;
        bool first_byte = false;
        bool second_byte = false;
        uint8_t expected_row = 0;
        int8_t first = 0;


        // Store data in buffer
        for (; bufferPosition < margin; bufferPosition++) {
          sample = Serial.read();
          
          // start of frame
          if (sample == 0x40 && !is_frame_started) { 
            is_frame_started = true;
          
          // frame number
          } else if (is_frame_started && (sample >= 1) && (sample <= 8) && !is_inside_frame) { 
            expected_row = sample;
            is_inside_frame = true;
            first_byte = true;
          
          // end of frame
          } else if (sample != 0x23) { 
            is_inside_frame = false;
            is_frame_started = false;
            continue;

          // read first byte
          } else if (first_byte) {
            first = sample;
            first_byte = false;
            second_byte = true;
          
          // read second byte and insert into buffer
          } else if (second_byte) {
            short value = short(
              (unsigned char)(first) << 8 |
              (unsigned char)(sample)
            );

            circularBuffer.insert(value, counter);
            counter += expected_row;

            if(counter >= 72) counter = 0;
            
            first_byte = true;
            second_byte = false;
          }
        }    
      }
    }

    // Send data end clear buffer
    diff = micros() - lastTimeSend;
    if (diff >= dataTimeSendMicrosec) {
      if (bufferPosition > 0) {
        base64::encode()
        int n = sprintf(message, "%u", circularBuffer.getCapacity());
        mqttClient.publish("test", message);
        lastTimeSend = micros();
        bufferPosition = 0;
        circularBuffer.clear();
      }
    }
  } 
  mqttReconnect();
}