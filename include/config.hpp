// build-in libraries 
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <limits>

// additional libraries
#include "PubSubClient.h"
#include "WiFi.h"

// debugging messages
#define DEBUG false

// network   
#define WIFI_NETWORK "HUAWEI-AE45B7"
#define WIFI_PASSWORD "J90D90L0MH3"
#define WIFI_TIMEOUT_MS 15000

// mqtt 
#define MQTT_SERVER_IP "192.168.8.105"
#define MQTT_SERVER_PORT 1883

// transmission 
#define SENDING_DATA_FREQ 50
#define BAUD_RATE 2000000
unsigned long dataTimeSendMicrosec = (1.0 / SENDING_DATA_FREQ) * 1000000;

// SERIAL
#define SERIAL_BUFFER_SIZE 10500
