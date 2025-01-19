#include "frameProtocol/uartFrame/UartFrame.hpp"

UartFrame::UartFrame()
{
}
UartFrame::~UartFrame()
{
}
std::shared_ptr<UartFrameData> UartFrame::create()
{
    return std::shared_ptr<UartFrameData>();
}