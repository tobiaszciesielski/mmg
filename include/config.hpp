// build-in libraries 
#include <Arduino.h>
#include <string>
#include <sstream>  
#include <chrono>
#include <random>
#include <ctime>

// additional libraries
#include "PubSubClient.h"
#include "WiFi.h"
#include "ArduinoJson.h"
#include "base64.h"

// external files
#include "circularMultiBuffer.hpp"

// debugging messages
#define DEBUG false

// network   
#define WIFI_NETWORK "Ciesielski Hot Spot"
#define WIFI_PASSWORD "Aneta1999"
#define WIFI_TIMEOUT_MS 15000

// mqtt 
#define MQTT_SERVER_IP "192.168.8.120"
#define MQTT_SERVER_PORT 1883

// transmission 
#define SENDING_DATA_FREQ 1000
#define BAUD_RATE 2000000
unsigned long messageSendingTime = (1.0 / SENDING_DATA_FREQ) * 1000;

// SERIAL
#define SERIAL_BUFFER_SIZE 256

