#include <stdint.h>
#include <memory>
#include <chrono>
#include <assert.h>
#include <vector>
#include "CRC16.hpp"
#include "FrameNumberHelper.hpp"
#include "setupConfiguration/utils.hpp"
#include "platform/platform.hpp"
#include "ellipticCurve/ecdh.hpp"
#include "communication/MQTT.hpp"
#include "asconEncryptionHelper/asconPrintstate.hpp"

#define UART_FRAME_MAX_DATA_SIZE 255
#define NUMBER_BYTE_OF_DATA 4

/**
 * @brief The data will be construct following this frame before sending to device or receive from device.
 */
namespace Transmission
{
namespace ServerFrame
{
namespace Handshake
{
typedef struct IGNORE_PADDING
{
    uint16_t s_preamble; // Sync bytes (e.g., 0xAA55)
    uint32_t s_identifierId; // Unique device/sensor ID
    uint8_t s_packetType; // 0x02 (Handshake)
    uint16_t s_sequenceNumber; // Monotonic counter for deduplication
    uint8_t s_publicKey[ECC_PUB_KEY_SIZE]; //Public key for encryption (32 bytes)
    uint16_t s_endMarker; // Integrity/authentication tag
} HandshakeFrameData;
}

namespace DataFrame
{
typedef struct IGNORE_PADDING
{
    uint16_t s_preamble;           // Sync bytes (e.g., 0xAA55)
    uint32_t s_identifierId;       // Unique device/sensor ID
    uint8_t s_packetType;          // 0x01 (DATA)
    uint16_t s_sequenceNumber = ServerFrameConstants::SERVER_FRAME_SEQUENCE_NUMBER;     // Monotonic counter for deduplication
    uint64_t s_timestamp;          // Timestamp (Unix epoch or device time)
    uint8_t s_nonce[NONCE_SIZE];           // Unique 128-bit nonce for encryption
    uint16_t s_payloadLength;      // Length of encrypted payload
    //Fix me, the allocation declaration not safe, redefine some variables
    uint8_t s_encryptedPayload[NUMBER_BYTE_OF_DATA + AUTH_TAG_SIZE]; // Encrypted data (Ascon-128)
    uint32_t s_macTag;       // Integrity/authentication tag
    uint16_t s_endMarker;
} ServerFrameData;
}

class ServerFrame
{
private:
    std::shared_ptr<Handshake::HandshakeFrameData> m_handshakeFrame;
    std::shared_ptr<DataFrame::ServerFrameData> m_serverDataFrame;
    uint16_t m_dataLength;
    uint16_t m_crcReceive;
    std::vector<uint8_t> m_frameBuffer;
    HandshakeState m_handshakeNextState;
    uint32_t m_lastByteTimestamp;
    bool m_isParsingActive = false;
    int sequenceNumber = 100;
    std::vector<unsigned char> m_secretKeyComputed;
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
     * @brief Smart pointer to create ServerFrameData object
     * @return A shared pointer to a new ServerFrameData object
     */
    static std::shared_ptr<ServerFrame> create();

    /**
     * @brief Perform handshake with server to acquire encryption key
     */
    void performHandshake(std::shared_ptr<MQTT> mqtt);

    /**
     * @brief Reset handshake state machine
     */
    void resetHandshakeState();

    /**
     * @brief Get the current handshake state
     * @return The current handshake state
     */
    HandshakeState getHandshakeState();

    /**
     * @brief Construct data frame
     * @param Nonce for setting the nonce packet
     */
    void constructServerDataFrame(const std::vector<unsigned char>& nonce, 
                                unsigned long long cipherTextLength, 
                                const std::vector<unsigned char>& cipherText);

    /**
     * @brief increase sequence number for each transmission
     */
    int currentSequenceNumber();

    /**
     * @brief send data frame to server using MQTT
     */
    void sendDataFrameToServer(std::shared_ptr<MQTT> mqtt,
                             const std::vector<unsigned char>& nonce,
                             unsigned long long ciphertextLength,
                             const std::vector<unsigned char>& ciphertext);
    
    /**
     * @brief return sequence number
     */
    uint16_t getSequenceNumber();

    /**
     * @brief Wait for ACK package
     */
    bool isAckFromServerArrived(std::shared_ptr<MQTT> mqtt);

    /**
     * Get secret key
     */
    std::vector<unsigned char>& getSecretKeyComputed();
};;
}
} // namespace Communication::UartFrame
