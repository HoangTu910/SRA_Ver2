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
    asdasdasdasda;
    std::string hexStr;
    m_mqttCallBackDataReceive.resize(length);
    for (unsigned int i = 0; i < length; i++) {
        char hex[4];
        m_mqttCallBackDataReceive[i] = payload[i];
        snprintf(hex, sizeof(hex), "%02X ", m_mqttCallBackDataReceive[i]); // Format as hex
        hexStr += hex;
    }
    // PLAT_LOG_D("-- Message arrived %s", hexStr.c_str());
    // PLAT_LOG_D("-- Length: %d", length);

    if (length == 0) {
        PLAT_LOG_D(__FMT_STR__, "-- Ignoring empty payload");
        return;
    }

    if(length == 1 && payload[0] == ServerFrameConstants::SERVER_FRAME_PACKET_ACK_TYPE){
        // PLAT_LOG_D("ACK package arrived [%d]", payload[0]);
        m_mqttIsAckPackageArrived = true;
    }
    else if(length == 1 && payload[0] == ServerFrameConstants::SERVER_FRAME_UPDATE_SEQUENCE_NUMBER){
        // PLAT_LOG_D("Sequence number update [%d]", payload[0]);
        m_mqttIsSequenceNumberNeededUpdate = true;
    }
    else{
        m_mqttIsMessageArrived = true;
    }
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
            PLAT_LOG_D("Subscribed to topics: %s", 
                      m_mqttPublicKeyReceiveTopic);
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
    if (!data) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: publishData received null data");
        return false;
    }
    try {
        bool result = m_client.publish(m_mqttPublicKeyTopic, (const uint8_t *)data, dataLength);
        PLAT_LOG_D(__FMT_STR__, result ? "-- Publish succeeded" : "-- Publish failed");
        return result;
    } catch (const std::exception& e) {
        PLAT_LOG_D("Publish exception: %s", e.what());
        return false;
    }
}

bool MQTT::publishInitSessionData(const void *data, size_t dataLength)
{
    if (!data) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: publishData received null data");
        return false;
    }
    try {
        const char *topic = "init/session";
        bool result = m_client.publish(topic, (const uint8_t *)data, dataLength);
        PLAT_LOG_D(__FMT_STR__, result ? "-- Publish succeeded" : "-- Publish failed");
        return result;
    } catch (const std::exception& e) {
        PLAT_LOG_D("Publish exception: %s", e.what());
        return false;
    }
}

bool MQTT::publishMetricsData(const void *data, size_t dataLength)
{
    if (!data) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: publishData received null data");
        return false;
    }
    try {
        const char *topic = "metrics/data";
        bool result = m_client.publish(topic, (const uint8_t *)data, dataLength);
        PLAT_LOG_D(__FMT_STR__, result ? "-- Publish succeeded" : "-- Publish failed");
        return result;
    } catch (const std::exception& e) {
        PLAT_LOG_D("Publish exception: %s", e.what());
        return false;
    }
}

std::shared_ptr<MQTT> MQTT::create(char *mqttServer, int mqttPort, char *mqttDeviceID, char *mqttDataTopic, char *mqttPublicKeyTopic, char *mqttUser, char *mqttPassword, char *mqttPublicKeyReceiveTopic)
{
    return std::make_shared<MQTT>(mqttServer, mqttPort, mqttDeviceID, mqttDataTopic, mqttPublicKeyTopic, mqttUser, mqttPassword, mqttPublicKeyReceiveTopic);
}


