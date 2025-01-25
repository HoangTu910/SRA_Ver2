#include "frameProtocol/uartFrame/UartFrame.hpp"
#include "frameProtocol/CRC16.hpp"

namespace Test
{
class UartFrameTest
{
    private:
        uint8_t mTest_headerHigh = 0xAB;
        uint8_t mTest_headerLow = 0xCD;
        uint8_t mTest_dataLenghtHigh = 0x00;
        uint8_t mTest_dataLenghtLow = 0x04;
        uint8_t mTest_data[4] = {0x01, 0x02, 0x03, 0x04};
        uint8_t mTest_trailerHigh = 0xE1;
        uint8_t mTest_trailerLow = 0xE2;
        uint8_t mTest_crcHigh;
        uint8_t mTest_crcLow;
        std::vector<uint8_t> mTest_frameBuffer;
    public:
        UartFrameTest();
        ~UartFrameTest();
        std::shared_ptr<UartFrameTest> m_uartFrameTest;
        static std::shared_ptr<UartFrameTest> create();
        static void frameParserTest();
};
}