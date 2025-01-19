#include <stdint.h>
#include <memory>
#include "CRC16.hpp"

#define UART_FRAME_MAX_DATA_SIZE 255

/**
 * @brief The data will be construct following this frame before sending to device or receive from device.
 */
typedef struct UartFrameData
{
    uint8_t str_headerHigh;
    uint8_t str_headerLow;
    uint16_t str_dataLenght;
    uint16_t str_data[UART_FRAME_MAX_DATA_SIZE];
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

    static std::shared_ptr<UartFrameData> create();
};


