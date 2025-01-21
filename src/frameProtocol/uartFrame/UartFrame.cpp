#include "frameProtocol/uartFrame/UartFrame.hpp"

namespace Transmission
{
namespace UartFrame
{

UartFrame::UartFrame()
{
}


UartFrame::~UartFrame()
{
}


void UartFrame::constructFrame()
{
    /**
     * @brief Currently not using this function
     * @brief The data frame will be sent by STM32 device
     * @brief Can be used to create mock data frame for transmission testing
     */
}

void UartFrame::parseFrame(uint8_t byteFrame)
{
    switch (m_parserNextState)
    {
        case UartParserState::VERIFY_HEADER_1_BYTE:
            m_parserNextState = isFirstHeaderByteValid(byteFrame) ? UartParserState::VERIFY_HEADER_2_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::VERIFY_HEADER_2_BYTE:
            m_parserNextState = isSecondHeaderByteValid(byteFrame) ? UartParserState::WAIT_FOR_DATA_LENGTH_1_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::WAIT_FOR_DATA_LENGTH_1_BYTE:
            m_parserNextState = isDataLengthFirstByteValid(byteFrame) ? UartParserState::WAIT_FOR_DATA_LENGTH_2_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::WAIT_FOR_DATA_LENGTH_2_BYTE:
            m_parserNextState = isDataLengthSecondByteValid(byteFrame) ? UartParserState::RECEIVE_DATA : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::RECEIVE_DATA:
            collectData(byteFrame);
            m_frameBuffer.size() == m_dataLength ? UartParserState::WAIT_FOR_CRC_1_BYTE : UartParserState::RECEIVE_DATA;
            break;
        case UartParserState::VERIFY_TRAILER_1_BYTE:
            m_parserNextState = isFirstTrailerByteValid(byteFrame) ? UartParserState::VERIFY_TRAILER_2_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::VERIFY_TRAILER_2_BYTE:
            m_parserNextState = isSecondTrailerByteValid(byteFrame) ? UartParserState::FRAME_COMPLETE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::WAIT_FOR_CRC_1_BYTE:
            m_parserNextState = isCrcFirstByteValid(byteFrame) ? UartParserState::WAIT_FOR_CRC_2_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::WAIT_FOR_CRC_2_BYTE:
            m_parserNextState = isCrcSecondByteValid(byteFrame) ? UartParserState::VERIFY_CRC : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::VERIFY_CRC:
            uint16_t crcCalculatedFromData = CRC16::calculateCRC(m_frameBuffer.data(), m_frameBuffer.size());
            m_parserNextState = isCrcMatched(crcCalculatedFromData) ? UartParserState::FRAME_COMPLETE : UartParserState::FRAME_ERROR;
            break;
    }
    m_parserFinalState = m_parserNextState;
}

void UartFrame::resetParserState()
{
    m_parserNextState = UartParserState::VERIFY_HEADER_1_BYTE;
}

void UartFrame::resetFrameBuffer()
{
    m_frameBuffer.clear();
}

bool UartFrame::isFirstHeaderByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_HEADER_1)
    {
        return true;
    }
    return false;
}

bool UartFrame::isSecondHeaderByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_HEADER_2)
    {
        return true;
    }
    return false;
}

bool UartFrame::isDataLengthFirstByteValid(uint8_t byteFrame)
{
    if (byteFrame <= UART_FRAME_MAX_DATA_SIZE)
    {
        m_dataLength = byteFrame;
        return true;
    }
    return false;
}

bool UartFrame::isDataLengthSecondByteValid(uint8_t byteFrame)
{
    if (byteFrame <= UART_FRAME_MAX_DATA_SIZE)
    {
        m_dataLength = (m_dataLength << 8) | byteFrame;
        return true;
    }
    return false;
}

bool UartFrame::isFirstTrailerByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_TRAILER_1)
    {
        return true;
    }
    return false;
}

bool UartFrame::isSecondTrailerByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_TRAILER_2)
    {
        return true;
    }
    return false;
}

bool UartFrame::isCrcFirstByteValid(uint8_t byteFrame)
{
    if (byteFrame <= BYTE_MAX)
    {
        m_crcReceive = byteFrame;
        return true;
    }
    return false;
}

bool UartFrame::isCrcSecondByteValid(uint8_t byteFrame)
{
    if (byteFrame <= BYTE_MAX)
    {
        m_crcReceive = (m_crcReceive << 8) | byteFrame;
        return true;
    }
    return false;
}

bool UartFrame::isCrcMatched(uint16_t crcCalculatedFromData)
{
    if (m_crcReceive == crcCalculatedFromData)
    {
        return true;
    }
    return false;
}

void UartFrame::collectData(uint8_t byteFrame)
{
    if(m_frameBuffer.size() == UART_FRAME_MAX_DATA_SIZE)
    {
        return;
    }
    m_frameBuffer.push_back(byteFrame);
}

std::shared_ptr<UartFrameData> UartFrame::create()
{
    return std::shared_ptr<UartFrameData>();
}
} 
} // namespace Transmission::UartFrame