// build-in libraries 
#include <Arduino.h>
#include <string.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <cstring>
#include<sstream>

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
unsigned long dataTimeSendMicrosec = (1.0 / SENDING_DATA_FREQ) * 1000000;

// SERIAL
#define SERIAL_BUFFER_SIZE 10500


template <typename T>
std::string to_string(T value)
{
    //create an output string stream
    std::ostringstream os ;

    //throw the value into the string stream
    os << value ;

    //convert the string stream into a string and return
    return os.str() ;
}

