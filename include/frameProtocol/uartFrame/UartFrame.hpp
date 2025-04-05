#include <stdint.h>
#include <memory>
#include <vector>
#include <chrono>
#include <Arduino.h>
#include "CRC16.hpp"
#include "FrameNumberHelper.hpp"
#include "setupConfiguration/utils.hpp"
#include "platform/platform.hpp"
#include "setupConfiguration/SetupNumberHelper.hpp"

#define UART_FRAME_MAX_DATA_SIZE 255
#define SECRET_KEY_SIZE 64
#define IDENTIFIER_ID_SIZE 4
#define SOF_SIZE 2
#define EOF_SIZE 2
#define NONCE_SIZE 16
#define AAD_MAX_SIZE_LEN 2
#define AAD_MAX_SIZE 5
#define AUTH_TAG_SIZE 16
#define SECRET_KEY_MAX_SIZE_LEN 2

#pragma once

/**
 * @brief The data will be construct following this frame before sending to device or receive from device.
 */
namespace Transmission
{
namespace UartFrame
{
typedef struct UartFrameData
{
    uint8_t str_headerHigh;
    uint8_t str_headerLow;
    uint8_t str_dataLenghtHigh;
    uint8_t str_dataLenghtLow;
    uint8_t str_data[UART_FRAME_MAX_DATA_SIZE];
    uint8_t str_trailerHigh;
    uint8_t str_trailerLow;
    uint8_t str_crcHigh;
    uint8_t str_crcLow;
} UartFrameData;

typedef struct IGNORE_PADDING UartFrameSTM32
{
    uint8_t str_header[SOF_SIZE];
    uint8_t str_packetType;
    uint8_t str_identifierId[IDENTIFIER_ID_SIZE];
    uint8_t str_nonce[NONCE_SIZE];
    uint8_t str_addLength[AAD_MAX_SIZE_LEN];
    uint8_t str_add[AAD_MAX_SIZE];
    uint8_t str_secretKeyLength[SECRET_KEY_MAX_SIZE_LEN];
    uint8_t str_secretKey[SECRET_KEY_SIZE];
    uint8_t str_authTag[AUTH_TAG_SIZE];
    uint8_t str_eof[EOF_SIZE];
} UartFrameSTM32;

typedef struct IGNORE_PADDING UartFrameSTM32Trigger
{
    uint8_t str_header[SOF_SIZE];
    uint8_t str_identifierId[IDENTIFIER_ID_SIZE];
    uint8_t str_triggerSignal;
    uint8_t str_addLength[AAD_MAX_SIZE_LEN];
    uint8_t str_add[AAD_MAX_SIZE];
    uint8_t str_eof[EOF_SIZE];
    uint8_t str_padding[98]; 
} UartFrameSTM32Trigger;

class UartFrame
{
private:
    /**
     * @brief Transmit data over UART
     * @param data Pointer to the data buffer to transmit
     * @param size Size of the data to transmit
     * @return true if transmission successful, false otherwise
     */
    bool UARTTransmitting(uint8_t* data, size_t size);
    
    std::shared_ptr<UartFrameData> m_uartFrame;
    std::shared_ptr<UartFrameSTM32> m_uartFrameSTM32;
    std::shared_ptr<UartFrameSTM32Trigger> m_uartFrameSTM32Trigger;
    uint16_t m_dataLength;
    uint16_t m_crcReceive;
    std::vector<uint8_t> m_frameBuffer;
    std::vector<uint8_t> m_deviceID;
    std::vector<uint8_t> m_nonceReceive;
    std::vector<uint8_t> m_frameReceiveBuffer;
    UartParserState m_parserNextState;
    UartParserState m_parserFinalState;
    uint32_t m_lastByteTimestamp;
    bool m_isParsingActive = false;
    bool m_isParsingComplete = false;
    HardwareSerial *m_uart = &Serial1;
public:
    const uint8_t IdentifierIDSTM[IDENTIFIER_ID_STM_SIZE] = {0x01, 0x02, 0x03, 0x04};
    /**
     * @brief Constructor of UartFrame
     */
    UartFrame();

    /**
     * @brief Destructor of UartFrame
     */
    ~UartFrame();

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

     * @brief Collect device ID
     * @param byteFrame The byte to collect
     */
    void collectDeviceID(uint8_t byteFrame);

    /**
     * @brief Collect nonce
     * @param byteFrame The byte to collect
     */
    void collectNonce(uint8_t byteFrame);

    /**
     * @brief Smart pointer to create UartFrameData object
     * @return A shared pointer to a new UartFrameData object
     */
    static std::shared_ptr<UartFrame> create();

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

    /**
     * @brief Get the final state of the parser
     * @return The final state to execute next step
     */
    UartParserState getFinalState();

    /**
     * @brief Get the data from the frame
     * @return The data from the frame
     */
    std::vector<uint8_t> getFrameBuffer();

    /**
     * @brief Begin uart communication
     */
    void beginUartCommunication();

    /**
     * @brief Update data
     */
    bool update();

    /**
     * @brief Check if parsing complete
     * @return True if parsing complete, false otherwise
     */
    bool isParsingComplete();


    // Merge 2 construct frame method into single one
    /**
     * @brief Structure to hold STM32 frame parameters
     */
    struct STM32FrameParams {
        std::vector<unsigned char> secretKey;
        std::vector<unsigned char> nonce;
        std::vector<unsigned char> aad;
        std::vector<unsigned char> authTag;
    } m_stm32FrameParams;

    /**
     * @brief Construct frame for transmitting key to STM32
     * @param params Structure containing all frame parameters
     */
    void constructFrameForTransmittingKeySTM32(const STM32FrameParams& params);


    /**
     * @brief Construct frame for transmitting trigger signal to STM32
     */
    void constructFrameForTransmittingTriggerSignal(std::vector<unsigned char> associatedData);

    /**
     * @brief Template function for transmitting data using UART
     * @tparam T Type of the struct containing data
     * @param data Reference to the struct containing data to transmit
     * @return true if transmission successful, false otherwise
     */
    template<typename T>
    bool transmitData(const T& data) {
        if (!m_uart) {
            return false;
        }
        
        const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(&data);
        size_t dataSize = sizeof(T);
        
        return UARTTransmitting(const_cast<uint8_t*>(dataPtr), dataSize);
    }

    /**
     * @brief Get the UartFrameSTM32 object
     * @return The UartFrameSTM32 object
     */
    std::shared_ptr<UartFrameSTM32> getUartFrameSTM32() {
        return m_uartFrameSTM32;
    }

    /**
     * @brief Get the UartFrameSTM32Trigger object
     * @return The UartFrameSTM32Trigger object
     */
    std::shared_ptr<UartFrameSTM32Trigger> getUartFrameSTM32Trigger() {
        return m_uartFrameSTM32Trigger;
    }
    
    /**
     * @brief Get nonce receive
     * @return The nonce receive
     */
    std::vector<uint8_t> getNonce();

    /**
     * @brief Get the frame buffer size
     * @return The frame buffer size
     */
    int getFrameBufferSize();
};
}
} // namespace Communication::UartFrame
