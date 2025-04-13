// communication_number_helper.cpp
#include "communication/CommunicationNumberHelper.hpp"

namespace WifiHelper
{
    char* SSID = (char*)"your_wifi_ssid";
    char* PASSWORD = (char*)"your_wifi_password";
    int WIFI_TIMEOUT = 5000;
}

namespace MQTTHelper
{   
    char* MQTT_SERVER = (char*)"192.168.1.24";
    int MQTT_PORT = 1885;
    char* MQTT_USER = (char*)"admin";
    char* MQTT_PASSWORD = (char*)"123";
    char* MQTT_DATA_TOPIC = (char*)"sensors/data";
    char* MQTT_PUBLIC_KEY_TOPIC = (char*)"handshake/ecdh";
    char* MQTT_DEVICE_ID = (char*)"newdevice123";
    char* MQTT_PUBLIC_KEY_RECEIVE_TOPIC = (char*)"handshake-send/ecdh";
    int MQTT_TIMEOUT = 5000;
}
