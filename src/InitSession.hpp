#pragma once  // Add header guard

#include "setupConfiguration/utils.hpp"
#include "communication/MQTT.hpp"
#include "communication/CommunicationNumberHelper.hpp"
#include "frameProtocol/uartFrame/UartFrame.hpp"
#include <memory>

#define SOF_SIZE 2
#define EOF_SIZE 2
#define IDENTIFIER_ID_SIZE 4

namespace InitSession {
typedef struct IGNORE_PADDING InitSessionFrame
{
    uint16_t str_header;
    uint8_t str_packetType;
    uint32_t str_identifierId;
    uint16_t str_eof;
} InitSessionFrame;

class InitSession {
public:
    InitSession();
    ~InitSession();
    std::vector<int> m_initialDataFromServer;
    // Declare functions but move definitions to cpp
    static std::shared_ptr<InitSession> create();
    int sendInitSessionFrame(std::shared_ptr<MQTT> mqtt);
    int isInitialDataFromServerReached(std::shared_ptr<MQTT> mqtt);
    int fetchSafeCounter(std::shared_ptr<MQTT> mqtt);
    int sendDeriviationIndexUart(std::shared_ptr<MQTT> mqtt,  std::shared_ptr<Transmission::UartFrame::UartFrame> m_uart);
};
}