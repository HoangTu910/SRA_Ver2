#include <stdint.h>
#include <memory>

#define UART_FRAME_MAX_DATA_SIZE 255

/**
 * @brief The data will be construct following this frame before sending to device or receive from device.
 */
typedef struct UartFrameData
{
    uint8_t str_headerHigh;
    uint8_t str_headerLow;
    uint16_t str_dataLenght;
    uint8_t str_data[UART_FRAME_MAX_DATA_SIZE];
    uint8_t str_trailerHigh;
    uint8_t str_trailerLow;
    uint16_t str_crc;
} UartFrameData;

class UartFrame
{
private:
    std::shared_ptr<UartFrameData> m_uartFrame;

public:
    UartFrame();
    ~UartFrame();
    void constructFrame();
    void parseFrame();
};

UartFrame::UartFrame() : m_uartFrame(std::make_shared<UartFrameData>())
{
    m_uartFrame->str_headerHigh = 0;
    m_uartFrame->str_headerLow = 0;
    m_uartFrame->str_dataLenght = 0;
    m_uartFrame->str_trailerHigh = 0;
    m_uartFrame->str_trailerLow = 0;
    m_uartFrame->str_crc = 0;
}

UartFrame::~UartFrame()
{
}
