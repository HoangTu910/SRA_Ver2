#include "InitSession.hpp"

namespace InitSession {

InitSession::InitSession() {
    PLAT_LOG_D(__FMT_STR__, "-- Init Session");
}

InitSession::~InitSession() {
    PLAT_LOG_D(__FMT_STR__, "-- Init Completed");
}

std::shared_ptr<InitSession> InitSession::create() {
    return std::make_shared<InitSession>();
}

int InitSession::sendInitSessionFrame(std::shared_ptr<MQTT> mqtt) {
    PLAT_LOG_D(__FMT_STR__, "-- Requesting initial data from server...");
    if (!mqtt) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: null MQTT pointer");
        return -1;
    }

    InitSessionFrame frame = {}; 
    std::vector<uint8_t> buffer;
    uint16_t header = SERVER_FRAME_PREAMBLE;
    uint32_t id = SERVER_FRAME_IDENTIFIER_ID;
    uint8_t packetType = SERVER_FRAME_PACKET_INIT_SESSION_TYPE;
    uint16_t trailer = SERVER_FRAME_END_MAKER;

    buffer.reserve(sizeof(InitSessionFrame));
    buffer.insert(buffer.end(), (uint8_t*)&header, (uint8_t*)&header + sizeof(header));
    buffer.insert(buffer.end(), (uint8_t*)&id, (uint8_t*)&id + sizeof(id));
    buffer.push_back(packetType);
    buffer.insert(buffer.end(), (uint8_t*)&trailer, (uint8_t*)&trailer + sizeof(trailer));

    return mqtt->publishData(buffer.data(), buffer.size()) ? 1 : 0;
}

int InitSession::isInitialDataFromServerReached(std::shared_ptr<MQTT> mqtt) {
    if (!mqtt) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: null MQTT pointer");
        return 0;
    }
    PLAT_LOG_D(__FMT_STR__, "-- Waiting for initial data from server...");

    unsigned long startTime = millis();
    const unsigned long timeout = 5000;

    while (!mqtt->m_mqttIsMessageArrived) {
        mqtt->m_client.loop(); 
        
        static unsigned long lastCheck = 0;
        if (!mqtt->m_client.connected() && (millis() - lastCheck >= 1000)) {
            lastCheck = millis();
            mqtt->connect();
            PLAT_LOG_D("MQTT State: %d", mqtt->m_client.state());
        }

        if (millis() - startTime >= timeout) {
            PLAT_LOG_D(__FMT_STR__, "-- Timeout waiting for initial data");
            return 0;
        }
        delay(10);
    }

    PLAT_LOG_D(__FMT_STR__, "-- Received initial data from server");
    if (mqtt->m_mqttCallBackDataReceive.empty()) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: Received empty data");
        return 0;
    }
    if(mqtt->m_mqttIsMessageArrived) {
        mqtt->m_mqttIsMessageArrived = false;  
        return 1;
    }
    else return 0;
}

int InitSession::fetchSafeCounter(std::shared_ptr<MQTT> mqtt) {
    if (!mqtt) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: null MQTT pointer");
        return -1;
    }

    PLAT_LOG_D(__FMT_STR__, "-- Fetching initial data from server...");
    
    if (mqtt->m_mqttCallBackDataReceive.empty()) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: No data received from server");
        return 0;
    }
    
    PLAT_LOG_D("-- Received safe counter: %d", mqtt->m_mqttCallBackDataReceive[0]);
    return static_cast<int>(mqtt->m_mqttCallBackDataReceive[0]); //2 byte
}

int InitSession::sendDeriviationIndexUart(std::shared_ptr<MQTT> mqtt,  std::shared_ptr<Transmission::UartFrame::UartFrame> m_uart) {
    if (!mqtt || !m_uart) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: Invalid pointers");
        return 0;
    }
    PLAT_LOG_D(__FMT_STR__, "-- Transmitting deriviation index to STM32...");
    std::vector<unsigned char> buffer(AAD_MAX_SIZE, 0);
    buffer[0] = mqtt->m_mqttCallBackDataReceive[1];
    buffer[1] = mqtt->m_mqttCallBackDataReceive[2];
    m_uart->constructFrameForTransmittingInititalData(buffer);
    auto frame = m_uart->m_uartFrameSTM32Init;
    m_uart->UARTTransmitting(reinterpret_cast<uint8_t*>(frame.get()), sizeof(Transmission::UartFrame::UartFrameSTM32Init));
    return 1;
}
}