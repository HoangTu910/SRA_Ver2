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
    m_mqttCallBackDataReceive.resize(length);
    for (unsigned int i = 0; i < length; i++) {
        char hex[4];
        m_mqttCallBackDataReceive[i] = payload[i];
        snprintf(hex, sizeof(hex), "%02X ", m_mqttCallBackDataReceive[i]); // Format as hex
        hexStr += hex;
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
    // PLAT_LOG_D("Message arrived [%s]", hexStr.c_str());
}

void MQTT::connect()
{
    static uint32_t lastConnectionAttempt = 0;
    static uint8_t retryCount = 0;
    const uint32_t CONNECTION_RETRY_INTERVAL = 5000;
    const uint8_t MAX_RETRIES = 3;
    
    uint32_t now = millis();

    // Check if disconnected and enough time has passed since last attempt
    if (!m_client.connected() && 
        (now - lastConnectionAttempt >= CONNECTION_RETRY_INTERVAL))
    {
        lastConnectionAttempt = now;

        // Log connection state
        PLAT_LOG_D("MQTT State: %d, Retry: %d", m_client.state(), retryCount);
        
        // Attempt connection with retry limit
        if (retryCount < MAX_RETRIES)
        {
            PLAT_LOG_D("%s", "Attempting MQTT connection...");
            
            // Ensure credentials are valid
            if (!m_mqttUser || !m_mqttPassword) {
                PLAT_LOG_D("%s", "Invalid MQTT credentials");
                return;
            }

            // Try to connect with timeout
            bool connected = false;
            try {
                connected = m_client.connect("ESP32Client", m_mqttUser, m_mqttPassword);
            }
            catch (const std::exception& e) {
                PLAT_LOG_D("MQTT connect exception: %s", e.what());
                retryCount++;
                return;
            }

            if (connected)
            {
                PLAT_LOG_D("%s", "MQTT Connected");
                retryCount = 0; // Reset retry counter on success
                
                // Subscribe with error checking
                if (!m_client.subscribe(m_mqttPublicKeyReceiveTopic)) {
                    PLAT_LOG_D("%s", "Subscribe failed");
                }
            }
            else
            {
                retryCount++;
                PLAT_LOG_D("Connection failed, attempts: %d", retryCount);
            }
        }
        else {
            PLAT_LOG_D("%s", "Max retries reached, waiting longer interval");
            lastConnectionAttempt = now + CONNECTION_RETRY_INTERVAL * 2; // Wait longer before next retry
            retryCount = 0; // Reset counter to allow future attempts
        }
    }

    // Process messages if connected
    if (m_client.connected()) {
        try {
            m_client.loop();
        }
        catch (const std::exception& e) {
            PLAT_LOG_D("MQTT loop exception: %s", e.what());
        }
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
    if (!data) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: publishData received null data");
        return false;
    }
    try {
        bool result = m_client.publish(m_mqttPublicKeyTopic, (const uint8_t *)data, dataLength);
        // PLAT_LOG_D(__FMT_STR__, result ? "-- Publish succeeded" : "-- Publish failed");
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


