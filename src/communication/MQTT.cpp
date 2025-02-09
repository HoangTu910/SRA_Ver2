#include "communication/MQTT.hpp"
#include "setupConfiguration/utils.hpp"
#include "MQTT.hpp"

MQTT::MQTT(char *mqttServer, int mqttPort, char *mqttDeviceID, char *mqttDataTopic, char *mqttPublicKeyTopic, char *mqttUser, char *mqttPassword, char *mqttPublicKeyReceiveTopic)
    : m_mqttServer(mqttServer),
      m_mqttPort(mqttPort),
      m_mqttDeviceID(mqttDeviceID),
      m_mqttDataTopic(mqttDataTopic),
      m_mqttPublicKeyTopic(mqttPublicKeyTopic),
      m_client(m_espClient) ,
      m_mqttUser(mqttUser),
      m_mqttPassword(mqttPassword),
      m_mqttPublicKeyReceiveTopic(mqttPublicKeyReceiveTopic)
{
    
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
    std::string hexStr;
    for (unsigned int i = 0; i < length; i++) {
        char hex[4];
        m_mqttCallBackDataReceive[i] = payload[i];
        snprintf(hex, sizeof(hex), "%02X ", m_mqttCallBackDataReceive[i]); // Format as hex
        hexStr += hex;
    }
    if(length == 1 && payload[0] == ServerFrameConstants::SERVER_FRAME_PACKET_ACK_TYPE){
        m_mqttIsAckPackageArrived = true;
    }
    else{
        m_mqttIsMessageArrived = true;
    }
    // PLAT_LOG_D("Message arrived [%s]", hexStr.c_str());
}
void MQTT::connect()
{
    if (!m_client.connected())
    {
        PLAT_LOG_D("%s", "Start MQTT connection...");
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
            PLAT_LOG_D("%s", "MQTT Connected");
            m_client.subscribe(m_mqttPublicKeyReceiveTopic);
        }
        else
        {
            PLAT_LOG_D("%s", "MQTT Failed! Have you started the MQTT service yet?");
            delay(MQTTHelper::MQTT_TIMEOUT);
        }
    }
}

void MQTT::setupServer()
{
    m_client.setServer(m_mqttServer, m_mqttPort);
    m_client.setCallback([this](char *topic, byte *payload, unsigned int length) { this->callBack(topic, payload, length); });
}

bool MQTT::publishData(const void *data, size_t dataLength)
{
    return m_client.publish(m_mqttPublicKeyTopic, (const uint8_t *)data, dataLength);
}

std::shared_ptr<MQTT> MQTT::create(char *mqttServer, int mqttPort, char *mqttDeviceID, char *mqttDataTopic, char *mqttPublicKeyTopic, char *mqttUser, char *mqttPassword, char *mqttPublicKeyReceiveTopic)
{
    return std::make_shared<MQTT>(mqttServer, mqttPort, mqttDeviceID, mqttDataTopic, mqttPublicKeyTopic, mqttUser, mqttPassword, mqttPublicKeyReceiveTopic);
}


