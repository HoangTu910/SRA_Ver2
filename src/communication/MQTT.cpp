#include "communication/MQTT.hpp"
#include "setupConfiguration/utils.hpp"

MQTT::MQTT()
{
}
MQTT::MQTT(char *mqttServer, int mqttPort, char *mqttDeviceID, char *mqttDataTopic, char *mqttPublicKeyTopic)
{
    m_mqttServer = mqttServer;
    m_mqttPort = mqttPort;
    m_mqttDeviceID = mqttDeviceID;
    m_mqttDataTopic = mqttDataTopic;
    m_mqttPublicKeyTopic = mqttPublicKeyTopic;
}
MQTT::~MQTT()
{
}
void MQTT::setMqttServer(char *mqttServer)
{
    m_mqttServer = mqttServer;
}
void MQTT::setMqttPort(int mqttPort)
{
    m_mqttPort = mqttPort;
}

void MQTT::setMqttDeviceID(char *mqttDeviceID)
{
    m_mqttDeviceID = mqttDeviceID;
}

void MQTT::setMqttDataTopic(char *mqttDataTopic)
{
    m_mqttDataTopic = mqttDataTopic;
}

void MQTT::setMqttPublicKeyTopic(char *mqttPublicKeyTopic)
{
    m_mqttPublicKeyTopic = mqttPublicKeyTopic;
}

void MQTT::setMqttClient()
{
    m_client = m_espClient;
}

void MQTT::callBack(char *topic, byte *payload, unsigned int length)
{
    // handle message arrived
}
void MQTT::mqttConfig()
{
    if (!m_client.connected())
    {
        PLAT_LOG_D("%s", "Attempting MQTT connection...");
        reconnect();
    }
    else
    {
        m_client.loop();
    }
}
void MQTT::reconnect()
{
    while(!m_client.connected())
    {
        PLAT_LOG_D("%s", "Attempting MQTT connection...");
        if(m_client.connect("ESP32Client", m_mqttUser, m_mqttPassword))
        {
            PLAT_LOG_D("%s", "Connected");
            m_client.subscribe(m_mqttPublicKeyTopic);
        }
        else
        {
            PLAT_LOG_D("%s", "Failed");
            delay(5000);
        }
    }
}