#include <stdint.h>
#include <memory>
#include <vector>
#include "CRC16.hpp"
#include "FrameNumberHelper.hpp"

#define UART_FRAME_MAX_DATA_SIZE 255

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
    UartParserState m_parserNextState;
    UartParserState m_parserFinalState;
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
     * @brief Check each byte of frame 
     */
    bool isFirstHeaderByteValid(uint8_t byteFrame);
    bool isSecondHeaderByteValid(uint8_t byteFrame);
    bool isDataLengthFirstByteValid(uint8_t byteFrame);
    bool isDataLengthSecondByteValid(uint8_t byteFrame);
    bool isFirstTrailerByteValid(uint8_t byteFrame);
    bool isSecondTrailerByteValid(uint8_t byteFrame);
    bool isCrcFirstByteValid(uint8_t byteFrame);
    bool isCrcSecondByteValid(uint8_t byteFrame);
    bool isCrcMatched(uint16_t crcReceive);
    /**
     * @brief Collect each byte of data
     */
    void collectData(uint8_t byteFrame);

    /**
     * @brief Smart pointer to create UartFrameData object
     */
    static std::shared_ptr<UartFrameData> create();
};
}
} // namespace Communication::UartFrame
