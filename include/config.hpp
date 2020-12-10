// build-in libraries 
#include <Arduino.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>

// additional libraries
#include "PubSubClient.h"
#include "WiFi.h"
#include "ArduinoJson.h"
#include "base64.h"

// external files
#include "circularMultiBuffer.hpp"

// debugging messages
#define DEBUG true

// network   
#define WIFI_NETWORK "biolab"
#define WIFI_PASSWORD "ALamakota12"
#define WIFI_TIMEOUT_MS 15000

// mqtt 
#define MQTT_SERVER_IP "192.168.1.106"
#define MQTT_SERVER_PORT 1883

// transmission 
#define SENDING_DATA_FREQ 50
#define BAUD_RATE 2000000
unsigned long dataTimeSendMicrosec = (1.0 / SENDING_DATA_FREQ) * 1000000;

// SERIAL
#define SERIAL_BUFFER_SIZE 10500
