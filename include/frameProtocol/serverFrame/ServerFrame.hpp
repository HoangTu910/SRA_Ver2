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
#include "asconCryptography/Ascon128a.hpp"

#define UART_FRAME_MAX_DATA_SIZE 255
#define NUMBER_BYTE_OF_DATA 4
#define ENCRYPT_PAYLOAD_SIZE 20

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
    uint16_t s_preamble;
    uint32_t s_identifierId;
    uint8_t s_packetType; 
    uint8_t s_nonce[NONCE_SIZE]; 
    uint8_t s_publicKeyLength;
    uint8_t s_publicKey[ECC_PUB_KEY_SIZE]; 
    uint8_t s_authTag[AUTH_TAG_SIZE];
    uint16_t s_endMarker; 
} HandshakeFrameData;
}

namespace DataFrame
{
typedef struct IGNORE_PADDING
{
    uint16_t s_preamble;           
    uint32_t s_identifierId;       
    uint8_t s_packetType;          
    int32_t s_sequenceNumber = ServerFrameConstants::INITIAL_SEQUENCE;             
    uint8_t s_nonce[NONCE_SIZE];           
    uint16_t s_payloadLength;      
    uint8_t s_encryptedPayload[ENCRYPT_PAYLOAD_SIZE];
    uint8_t s_authTag[AUTH_TAG_SIZE];     
    uint16_t s_endMarker;
} ServerFrameData;
}

class ServerFrame
{
private:
    std::shared_ptr<Handshake::HandshakeFrameData> m_handshakeFrame;
    std::shared_ptr<DataFrame::ServerFrameData> m_serverDataFrame;
    std::shared_ptr<Cryptography::Ascon128a> m_ascon128a;
    uint16_t m_dataLength;
    uint16_t m_crcReceive;
    std::vector<uint8_t> m_frameBuffer;
    HandshakeState m_handshakeNextState;
    uint32_t m_lastByteTimestamp;
    bool m_isParsingActive = false;
    int m_safeCounter = 0;
    int sequenceNumber = 100;
    std::vector<unsigned char> m_secretKeyComputed;
    std::vector<unsigned char> m_presharedKeyForAuthTag = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18, 0x29, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x90};
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
                                const std::vector<unsigned char>& cipherText,
                                const std::vector<unsigned char>& authTag);

    /**
     * @brief increase sequence number for each transmission
     */
    void currentSequenceNumber();

    /**
     * @brief send data frame to server using MQTT
     */
    void sendDataFrameToServer(std::shared_ptr<MQTT> mqtt,
                             const std::vector<unsigned char>& nonce,
                             unsigned long long ciphertextLength,
                             const std::vector<unsigned char>& ciphertext,
                             const std::vector<unsigned char>& authTag);
    
    /**
     * @brief return sequence number
     */
    int16_t getSequenceNumber();

    /**
     * @brief Wait for ACK package
     */
    int isPacketFromServerReached(std::shared_ptr<MQTT> mqtt);

    /**
     * Get secret key
     */
    std::vector<unsigned char>& getSecretKeyComputed();
    
    /**
     * @brief Reset sequence number
     */
    void resetSequenceNumber();

    /**
     * @brief Get safe counter
     */
    int getSafeCounter();

    /**
     * @brief Set safe counter
     */
    void setSafeCounter(int safeCounter);

    /**
     * @brief Set sequence number
     */
    void setSequenceNumber(int sequenceNumber);
};
} 
} // namespace Communication::UartFrame
