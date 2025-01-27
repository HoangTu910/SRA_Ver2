#include "frameProtocol/uartFrame/UartFrame.hpp"

namespace Transmission
{
namespace UartFrame
{

UartFrame::UartFrame()
{
    resetStateMachine();
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
    m_lastByteTimestamp = Platform::GetCurrentTimeMs();
    m_isParsingActive = true;
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
            m_parserNextState = (m_frameBuffer.size() == m_dataLength) ? UartParserState::VERIFY_TRAILER_1_BYTE : UartParserState::RECEIVE_DATA;
            break;
        case UartParserState::VERIFY_TRAILER_1_BYTE:
            m_parserNextState = isFirstTrailerByteValid(byteFrame) ? UartParserState::VERIFY_TRAILER_2_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::VERIFY_TRAILER_2_BYTE:
            m_parserNextState = isSecondTrailerByteValid(byteFrame) ? UartParserState::WAIT_FOR_CRC_1_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::WAIT_FOR_CRC_1_BYTE:
            m_parserNextState = isCrcFirstByteValid(byteFrame) ? UartParserState::WAIT_FOR_CRC_2_BYTE : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::WAIT_FOR_CRC_2_BYTE:
            m_parserNextState = isCrcSecondByteValid(byteFrame) ? UartParserState::VERIFY_CRC : UartParserState::FRAME_ERROR;
            break;
        case UartParserState::VERIFY_CRC: {
            uint16_t crcCalculatedFromData = CRC16::calculateCRC(m_frameBuffer.data(), m_frameBuffer.size());
            m_parserNextState = isCrcMatched(crcCalculatedFromData) ? UartParserState::FRAME_COMPLETE : UartParserState::FRAME_ERROR;
            break;
        }
        case UartParserState::FRAME_COMPLETE:
            // processCompleteFrame();
            resetStateMachine();
            break;
        case UartParserState::FRAME_ERROR:
            // handleFrameError();
            break;
        default:
            resetStateMachine();
            break;
    }
    m_parserFinalState = m_parserNextState;
}

void UartFrame::resetParserState()
{
    m_parserNextState = UartParserState::VERIFY_HEADER_1_BYTE;
    m_parserFinalState = UartParserState::VERIFY_HEADER_1_BYTE;
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
    PLAT_LOG_D("[HEADER_1 FAILED] actual: %d, expect: %d", byteFrame, UartFrameConstants::UART_FRAME_HEADER_1);
    return false;
}

bool UartFrame::isSecondHeaderByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_HEADER_2)
    {
        return true;
    }
    PLAT_LOG_D("[HEADER_2 FAILED] actual: %d, expect: %d", byteFrame, UartFrameConstants::UART_FRAME_HEADER_2);
    return false;
}

bool UartFrame::isDataLengthFirstByteValid(uint8_t byteFrame)
{
    if (byteFrame <= UART_FRAME_MAX_DATA_SIZE)
    {
        m_dataLength = byteFrame;
        return true;
    }
    PLAT_LOG_D("[DATA_LENGHT_1 FAILED] actual: %d, expect: <= %d", byteFrame, UART_FRAME_MAX_DATA_SIZE);
    return false;
}

bool UartFrame::isDataLengthSecondByteValid(uint8_t byteFrame)
{
    if (byteFrame <= UART_FRAME_MAX_DATA_SIZE)
    {
        m_dataLength = (m_dataLength << 8) | byteFrame;
        return true;
    }
    PLAT_LOG_D("[DATA_LENGHT_2 FAILED] actual: %d, expect: <= %d", byteFrame, UART_FRAME_MAX_DATA_SIZE);
    return false;
}

bool UartFrame::isFirstTrailerByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_TRAILER_1)
    {
        return true;
    }
    PLAT_LOG_D("[TRAILER_1 FAILED] actual: %d, expect: %d", byteFrame, UartFrameConstants::UART_FRAME_TRAILER_1);
    return false;
}

bool UartFrame::isSecondTrailerByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_TRAILER_2)
    {
        return true;
    }
    PLAT_LOG_D("[TRAILER_2 FAILED] actual: %d, expect: %d", byteFrame, UartFrameConstants::UART_FRAME_TRAILER_2);
    return false;
}

bool UartFrame::isCrcFirstByteValid(uint8_t byteFrame)
{
    if (byteFrame <= BYTE_MAX)
    {
        m_crcReceive = byteFrame;
        return true;
    }
    PLAT_LOG_D("[BYTE FAILED] actual: %d, expect: <= %d", byteFrame, BYTE_MAX);
    return false;
}

bool UartFrame::isCrcSecondByteValid(uint8_t byteFrame)
{
    if (byteFrame <= BYTE_MAX)
    {
        m_crcReceive = (m_crcReceive << 8) | byteFrame;
        return true;
    }
    PLAT_LOG_D("[BYTE FAILED] actual: %d, expect: <= %d", byteFrame, BYTE_MAX);
    return false;
}

bool UartFrame::isCrcMatched(uint16_t crcCalculatedFromData)
{
    if (m_crcReceive == crcCalculatedFromData)
    {
        return true;
    }
    PLAT_LOG_D("[CRC FAILED] actual: %d, expect: %d", m_crcReceive, crcCalculatedFromData);
    return false;
}

void UartFrame::collectData(uint8_t byteFrame) {
    if (m_frameBuffer.size() < UART_FRAME_MAX_DATA_SIZE) {
        m_frameBuffer.push_back(byteFrame);
    } else {
        PLAT_ASSERT((m_frameBuffer.size() < UART_FRAME_MAX_DATA_SIZE), 
                    "[BUFFER OVERFLOW] Max data size reached: %d", 
                        UART_FRAME_MAX_DATA_SIZE);
        m_parserNextState = UartParserState::FRAME_ERROR;
    }
}

bool UartFrame::parseFrame(std::vector<uint8_t> byteBuffer)
{
    for(uint8_t i : byteBuffer)
    {
        parseFrame(i);
        checkTimeout();
    }
    while (m_parserNextState != UartParserState::FRAME_COMPLETE && m_parserNextState != UartParserState::FRAME_ERROR)
    {
        parseFrame(Dummy::DUMMY); // Pass dummy byte to continue state machine
        checkTimeout();
    }
    // PLAT_LOG_D("[PARSER STATE] %d", static_cast<int>(m_parserFinalState));
    return m_parserFinalState == UartParserState::FRAME_COMPLETE;
}

void UartFrame::checkTimeout()
{
    if (!m_isParsingActive) return;  
    uint32_t currentTime = Platform::GetCurrentTimeMs();
    if ((currentTime - m_lastByteTimestamp) >= UartTimer::UART_FRAME_TIMEOUT_MS) {
        PLAT_LOG_D(__FMT_STR__,"Timeout! Resetting parser.");
        resetStateMachine();
        m_isParsingActive = false;
    }
}

UartParserState UartFrame::getFinalState()
{
    return m_parserFinalState;
}

std::vector<uint8_t> UartFrame::getFrameBuffer()
{
    return m_frameBuffer;
}
void UartFrame::resetStateMachine()
{
    resetFrameBuffer();
    resetParserState();
    m_dataLength = 0;
    m_crcReceive = 0;
}

void UartFrame::handleFrameError()
{
    PLAT_LOG_D("[FRAME ERROR] Parsing failed at state: %d", static_cast<int>(m_parserFinalState));
    resetStateMachine();
}

std::shared_ptr<UartFrame> UartFrame::create() 
{
    return std::make_shared<UartFrame>();
}
} 
} // namespace Transmission::UartFrame