#include "../include/config.hpp"



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

void sendJson(const char* json) {
  mqttClient.publish("test", json);
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
  mqttClient.setBufferSize(2200);
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
}

// policzyć moduły przyspieszeń

void loop() {

  // =====================
  //  necessary variables
  // =====================
  const uint16_t buffSize = 900;

  unsigned long lastTimeSend = 0;
  unsigned long diff;

  int aviableBytes = 0;

  char json[2000];

  int aviablePackages = 0;

  unsigned long timeField = 0;
  char tmp[11];

  String encodedFrame;
  encodedFrame.reserve(24);

  char freq[6];
  freq[0]='\0';
  my::itoa(SENDING_DATA_FREQ, freq, 10);

  // ========
  //  buffer
  // ========
  const size_t packagesCount = 8, packageSize = 8, frameSize = 18;  
  uint8_t** package = new uint8_t*[packageSize];
  for(int8_t i = 0; i < packageSize; ++i)
    package[i] = new uint8_t[frameSize];
  
  // buffer is 3d array which means we have got many packages 
  // which every package consinsts of `packageSize` frames. 
  // Every frame has length of `frameSize`. 
  Buffer<uint8_t> buffer(packagesCount, packageSize, frameSize);

  // ===============
  //  state machine
  // ===============
  enum class States { start, nr, data, eof };
  States state = States::start;
  uint8_t value = 0;
  int position = 0;
  size_t frameIndex = 0;
  std::vector<uint8_t> frame(frameSize, 0);

  // Start transmission transmission of HEX-type data (0x21 is '!')
  Serial.write(0x21);

  while (mqttClient.connected()) {
    mqttClient.loop();

    // Read data from serial
    aviableBytes = Serial.available();
    if (aviableBytes > 0) {
  
      // Prevent buffer from overlow and serial blocking
      if (aviableBytes >= buffSize) {
        for (int i = 0; i < aviableBytes; i++) {
          Serial.read();
        }
        sendData((const uint8_t*)"Buffer overflow", 16);
      } else {
        for (int i = 0; i < aviableBytes; i++) {
          value = Serial.read();

          switch (state) {
            case States::start:
              if (value == 0x40)
                state = States::nr;
              break;

            case States::nr:
              if (value >= 0x01 && value <= 0x08){
                state = States::data;
                frameIndex = value - 1;
              } else {
                state = States::start;
              }
              break;

            case States::data:
              frame[position] = value;
              position++;
              if(position == frameSize) {
                position = 0;
                state = States::eof;
              }
              break;
            
            case States::eof:
              if (value == 0x23) {
                buffer.insert(frame, frameIndex);
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

    // Send data
    diff = micros() - lastTimeSend;
    if (diff >= dataTimeSendMicrosec) {
      aviablePackages = buffer.getCapacity();
      if (aviablePackages > 0) {
       
        json[0] = '\0';
        char *p = json;

        // begin json
        p = my::strcat(p, "{\"data\":[");
        for (size_t i = 0; i < aviablePackages; i++) {
          buffer.get(package);
          p = my::strcat(p, "[");
          for (int j = 0; j < 8; j++) {
            encodedFrame = base64::encode(package[j], 18);
            p = my::strcat(p, "\"");
            p = my::strcat(p, (char*)encodedFrame.c_str());
            p = my::strcat(p, "\"");
            if (j < 7) p = my::strcat(p, ",");
          }
          p = my::strcat(p, "]");
          if (i < aviablePackages-1) p = my::strcat(p, ",");
        }
        p = my::strcat(p, "]");
        
        // "packets" field
        p = my::strcat(p, ",\"packets\":");
        tmp[0] = '\0';
        my::itoa(aviablePackages, tmp, 10);
        p = my::strcat(p, tmp);

        // "timestamp" field
        p = my::strcat(p, ",\"timestamp\":");
        tmp[0] = '\0';
        timeField = micros();
        my::itoa(timeField, tmp, 10);
        p = my::strcat(p, tmp);

        // // "diffrence" field
        // p = my::strcat(p, ",\"diffrence\":");
        // tmp[0] = '\0';
        // timeField = diff;
        // my::itoa(timeField, tmp, 10);
        // p = my::strcat(p, tmp);

        // "freq" field
        p = my::strcat(p, ",\"freq\":");
        p = my::strcat(p, freq);

        // finish json
        p = my::strcat(p, "}");
        *p = '\0';

        sendJson(json);

        lastTimeSend = micros();
      }
    }
  } 
  mqttReconnect();
}