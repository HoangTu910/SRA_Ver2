#include "test/frameProtocol/uartFrame/uartFrameTest.hpp"

namespace Test{
UartFrameTest::UartFrameTest()
{
    uint16_t crc16 = CRC16::calculateCRC(mTest_data, 4);
    mTest_crcHigh = (crc16 >> 8) & 0xFF;  
    mTest_crcLow = crc16 & 0xFF;

    mTest_frameBuffer = {
        mTest_headerHigh,
        mTest_headerLow,
        mTest_dataLenghtHigh,
        mTest_dataLenghtLow,
        mTest_data[0],
        mTest_data[1],
        mTest_data[2],
        mTest_data[3],
        mTest_trailerHigh,
        mTest_trailerLow,
        mTest_crcHigh,
        mTest_crcLow
    };
}

UartFrameTest::~UartFrameTest()
{
}
std::shared_ptr<UartFrameTest> Test::UartFrameTest::create()
{
    return std::make_shared<UartFrameTest>();
}

void Test::UartFrameTest::frameParserTest()
{
    auto testObj = UartFrameTest::create();
    PLAT_ASSERT_NULL(testObj, __FMT_STR__, "testObj is uninitilized");
    
    auto uartFrame = Transmission::UartFrame::UartFrame::create();
    PLAT_ASSERT_NULL(uartFrame, __FMT_STR__, "uartFrame is uninitilized");
    
    bool testResult = uartFrame->parseFrame(testObj->mTest_frameBuffer);
    TEST_RESULT(testResult);
}
}
