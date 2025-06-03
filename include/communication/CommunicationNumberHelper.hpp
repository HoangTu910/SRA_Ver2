#ifndef COMMUNICATION_NUMBER_HELPER_HPP
#define COMMUNICATION_NUMBER_HELPER_HPP

namespace WifiHelper
{
    extern char *SSID;
    extern char *PASSWORD;
}

namespace MQTTHelper
{   
    extern char* MQTT_SERVER;
    extern int MQTT_PORT;
    extern char* MQTT_USER;
    extern char* MQTT_PASSWORD;
    extern char* MQTT_DATA_TOPIC;
    extern char* MQTT_PUBLIC_KEY_TOPIC;
    extern char* MQTT_PUBLIC_KEY_RECEIVE_TOPIC;
    extern char* MQTT_DEVICE_ID;
    extern char* MQTT_INIT_SESSION_TOPIC;
    extern int MQTT_TIMEOUT;
}

#endif // COMMUNICATION_NUMBER_HELPER_HPP
