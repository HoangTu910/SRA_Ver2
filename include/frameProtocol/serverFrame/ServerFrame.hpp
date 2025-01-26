#include <stdint.h>
#include <memory>
#include <vector>
#include "CRC16.hpp"
#include "FrameNumberHelper.hpp"
#include "setupConfiguration/utils.hpp"
#include "platform/platform.hpp"

#define UART_FRAME_MAX_DATA_SIZE 255

/**
 * @brief The data will be construct following this frame before sending to device or receive from device.
 */
namespace Transmission
{
namespace ServerFrame
{
namespace Handshake
{
typedef struct HandshakeFrameData
{
    uint16_t s_preamble; // Sync bytes (e.g., 0xAA55)
    uint32_t s_identifierId; // Unique device/sensor ID
    uint8_t s_packetType; // 0x02 (Handshake)
    uint16_t s_sequenceNumber; // Monotonic counter for deduplication
    uint8_t s_publicKey[PUBLIC_KEY_SIZE]; //Public key for encryption (32 bytes)
    uint8_t s_authTag[AUTH_TAG_SIZE]; // Integrity/authentication tag
} HandshakeFrameData;
}

namespace DataFrame
{
typedef struct ServerFrameData
{
    uint16_t s_preamble;           // Sync bytes (e.g., 0xAA55)
    uint32_t s_identifierId;       // Unique device/sensor ID
    uint8_t s_packetType;          // 0x01 (DATA)
    uint16_t s_sequenceNumber;     // Monotonic counter for deduplication
    uint64_t s_timestamp;          // Timestamp (Unix epoch or device time)
    uint8_t s_nonce[NONCE_SIZE];           // Unique 128-bit nonce for encryption
    uint16_t s_payloadLength;      // Length of encrypted payload (0-1024 bytes)
    uint8_t s_encryptedPayload[ENCRYPTED_PAYLOAD_SIZE]; // Encrypted data (Ascon-128)
    uint8_t s_authTag[AUTH_TAG_SIZE];         // Integrity/authentication tag
} ServerFrameData;
}

class ServerFrame
{
private:
    std::shared_ptr<Handshake::HandshakeFrameData> m_handshakeFrame;
    std::shared_ptr<DataFrame::ServerFrameData> m_serverFrame;
    uint16_t m_dataLength;
    uint16_t m_crcReceive;
    std::vector<uint8_t> m_frameBuffer;
    UartParserState m_parserNextState;
    UartParserState m_parserFinalState;
    uint32_t m_lastByteTimestamp;
    bool m_isParsingActive = false;
public:
    /**
     * @brief Constructor of UartFrame
     */
    ServerFrame();

    /**
     * @brief Destructor of UartFrame
     */
    ~ServerFrame();

    /**
     * @brief Construct frame from data
     */
    void constructFrame();

    /**
     * @brief Parse each byte of frame and verify the frame
     * @param byteFrame The byte to parse
     */
    void parseFrame(uint8_t byteFrame);

    /**
     * @brief Reset parser state
     */
    void resetParserState();

    /**
     * @brief Reset frame buffer
     */
    void resetFrameBuffer();

    /**
     * @brief Check if the first header byte is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isFirstHeaderByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the second header byte is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isSecondHeaderByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the first byte of data length is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isDataLengthFirstByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the second byte of data length is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isDataLengthSecondByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the first trailer byte is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isFirstTrailerByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the second trailer byte is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isSecondTrailerByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the first CRC byte is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isCrcFirstByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the second CRC byte is valid
     * @param byteFrame The byte to check
     * @return True if valid, false otherwise
     */
    bool isCrcSecondByteValid(uint8_t byteFrame);

    /**
     * @brief Check if the received CRC matches the calculated CRC
     * @param crcReceive The received CRC value
     * @return True if matched, false otherwise
     */
    bool isCrcMatched(uint16_t crcReceive);

    /**
     * @brief Collect each byte of data
     * @param byteFrame The byte to collect
     */
    void collectData(uint8_t byteFrame);

    /**
     * @brief Smart pointer to create UartFrameData object
     * @return A shared pointer to a new UartFrameData object
     */
    static std::shared_ptr<ServerFrame> create();

    /**
     * @brief Reset the state machine
     */
    void resetStateMachine();

    /**
     * @brief Handle frame error
     */
    void handleFrameError();

    /**
     * @brief This function is used for testing purpose
     */
    bool parseFrame(std::vector<uint8_t> byteBuffer);

    /**
     * @brief Check timeout
     */
    void checkTimeout();
};
}
} // namespace Communication::UartFrame
