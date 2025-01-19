#include <stdint.h>
#include <memory>

#define SERVER_FRAME_MAX_DATA_SIZE 1024

/**
 * @brief The data will be construct following this frame before sending to server.
 */
typedef struct ServerFrameData
{
    uint16_t str_preamble;
    uint32_t str_identifier;
    uint8_t str_packetType;
    uint16_t str_sequenceNumber;
    uint32_t str_timestamp;
    uint16_t str_payloadLength;
    int str_payload[SERVER_FRAME_MAX_DATA_SIZE];
    uint16_t str_crc;
} ServerFrameData;


class ServerFrame
{
private:
    std::shared_ptr<ServerFrameData> m_serverFrame;

public:
    ServerFrame();
    ~ServerFrame();
    void constructFrame();
    void parseFrame();
};

ServerFrame::ServerFrame() : m_serverFrame(std::make_shared<ServerFrameData>())
{
    m_serverFrame->str_preamble = 0;
    m_serverFrame->str_identifier = 0;
    m_serverFrame->str_packetType = 0;
    m_serverFrame->str_sequenceNumber = 0;
    m_serverFrame->str_timestamp = 0;
    m_serverFrame->str_payloadLength = 0;
    m_serverFrame->str_crc = 0;
}

ServerFrame::~ServerFrame()
{
}
