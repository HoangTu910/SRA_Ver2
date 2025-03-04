#include <stdint.h>
#include <memory>
#include <vector>
#include <Arduino.h>
#include "CRC16.hpp"
#include "FrameNumberHelper.hpp"
#include "setupConfiguration/utils.hpp"
#include "platform/platform.hpp"
#include "setupConfiguration/SetupNumberHelper.hpp"

#define UART_FRAME_MAX_DATA_SIZE 255
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

class UartFrame
{
private:
    std::shared_ptr<UartFrameData> m_uartFrame;
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
};
}
} // namespace Communication::UartFrame
