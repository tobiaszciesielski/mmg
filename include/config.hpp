#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <base64.h>
#include "circularMultiBuffer.hpp"
#include "myFunctions.hpp"

// additional libraries
#include "PubSubClient.h"
#include "WiFi.h"

// debugging messages
#define DEBUG true

// network   
// #define WIFI_NETWORK "Ciesielski Hot Spot"
// #define WIFI_PASSWORD "Aneta1999"
#define WIFI_NETWORK "Ciesielski_Hot_Spot"
#define WIFI_PASSWORD "Aneta1999"
#define WIFI_TIMEOUT_MS 15000

// mqtt 
#define MQTT_SERVER_IP "192.168.1.26"
#define MQTT_SERVER_PORT 1883

// transmission 
#define SENDING_DATA_FREQ 50
#define BAUD_RATE 2000000
unsigned long dataTimeSendMicrosec = (1.0 / SENDING_DATA_FREQ) * 1000000;

// SERIAL
#define SERIAL_BUFFER_SIZE 12000

namespace topic {
    const char data[] = "sensors/mmg/data";
    const char control[] = "sensors/mmg/control";
    const char log[] = "sensors/mmg/log";
}

const char startStreamingCommand[] = "start";
const char stopStreamingCommand[] = "stop";

bool isDataStreaming = false;
