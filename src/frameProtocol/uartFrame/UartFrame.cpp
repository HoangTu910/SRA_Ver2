#include "frameProtocol/uartFrame/UartFrame.hpp"

namespace Transmission
{
namespace UartFrame
{

UartFrame::UartFrame()
{
    beginUartCommunication();
    resetStateMachine();
    m_uartFrameSTM32 = std::make_shared<UartFrameSTM32>();
    m_uartFrameSTM32Trigger = std::make_shared<UartFrameSTM32Trigger>();
}

UartFrame::~UartFrame()
{
}

bool UartFrame::UARTTransmitting(uint8_t* data, size_t size) {
    if (!m_uart || !data || size == 0) {
        return false;
    }

    size_t bytesWritten = m_uart->write(data, size);
    // for(int i = 0; i < size; i++) {
    //     PLAT_LOG_D("[UART TX %d] Data: %d", i, data[i]);
    // }
    m_uart->flush(); 

    if (bytesWritten != size) {
        PLAT_LOG_D("[UART TX ERROR] Bytes written: %d, Expected: %d", bytesWritten, size);
        return false;
    }

    PLAT_LOG_D("-- Transmission successful %d bytes", bytesWritten);
    return true;
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
        m_parserNextState = isSecondHeaderByteValid(byteFrame) ? UartParserState::RECEIVE_DEVICE_ID : UartParserState::FRAME_ERROR;
        break;
    case UartParserState::RECEIVE_DEVICE_ID:
        collectDeviceID(byteFrame);
        m_parserNextState = (m_deviceID.size() == Device::BYTE_LENGTH) ? UartParserState::RECEIVE_NONCE : UartParserState::RECEIVE_DEVICE_ID;
        break;
    case UartParserState::RECEIVE_NONCE:
        collectNonce(byteFrame);
        m_parserNextState = (m_nonceReceive.size() == NONCE_SIZE) ? UartParserState::WAIT_FOR_DATA_LENGTH_1_BYTE : UartParserState::RECEIVE_NONCE;
        break;
    case UartParserState::WAIT_FOR_DATA_LENGTH_1_BYTE:
        m_parserNextState = isDataLengthFirstByteValid(byteFrame) ? UartParserState::WAIT_FOR_DATA_LENGTH_2_BYTE : UartParserState::FRAME_ERROR;
        break;
    case UartParserState::WAIT_FOR_DATA_LENGTH_2_BYTE:
        m_parserNextState = isDataLengthSecondByteValid(byteFrame) ? UartParserState::RECEIVE_DATA : UartParserState::FRAME_ERROR;
        break;
    case UartParserState::RECEIVE_DATA:
        collectData(byteFrame);
        m_parserNextState = (m_frameBuffer.size() == m_dataLength) ? UartParserState::RECEIVE_AUTH_TAG : UartParserState::RECEIVE_DATA;
        break;
    case UartParserState::RECEIVE_AUTH_TAG:
        collectAuthTag(byteFrame);
        m_parserNextState = (m_authTag.size() == AUTH_TAG_SIZE) ? UartParserState::VERIFY_TRAILER_1_BYTE : UartParserState::RECEIVE_AUTH_TAG;
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
    case UartParserState::VERIFY_CRC:
    {
        uint16_t crcCalculatedFromData = CRC16::calculateCRC(m_frameBuffer.data(), m_frameBuffer.size());
        m_parserNextState = isCrcMatched(crcCalculatedFromData) ? UartParserState::FRAME_COMPLETE : UartParserState::FRAME_ERROR;
        break;
    }
    case UartParserState::FRAME_COMPLETE:
        // processCompleteFrame();
        // resetStateMachine();
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
    m_frameReceiveBuffer.clear();
    m_deviceID.clear();
    m_nonceReceive.clear();
    m_authTag.clear();
}

bool UartFrame::isFirstHeaderByteValid(uint8_t byteFrame)
{
    switch (byteFrame) {
        case UartFrameConstants::UART_FRAME_HEADER_1:
            // PLAT_LOG_D("[HEADER_1 PASSED] %d", byteFrame);
            return true;
            
        case UartFrameConstants::UART_FRAME_ERROR_ENCRYPTED:
            PLAT_LOG_D(__FMT_STR__, "[ENCRYPTED FRAME ERROR]");
            return false;
            
        case UartFrameConstants::UART_FRAME_ERROR_UNKNOWN:
            PLAT_LOG_D(__FMT_STR__, "[UNKNOWN FRAME ERROR]");
            return false;
            
        case UartFrameConstants::UART_FRAME_ERROR_MISMATCH:
            PLAT_LOG_D(__FMT_STR__, "[MISMATCH FRAME ERROR]");
            return false;
            
        case UartFrameConstants::UART_FRAME_ERROR_IDENTIFIER:
            PLAT_LOG_D(__FMT_STR__, "[IDENTIFIER FRAME ERROR]");
            return false;

        case UartFrameConstants::UART_FRAME_ERROR_HEADER_MISMATCH:
            PLAT_LOG_D(__FMT_STR__, "[HEADER MISMATCH FRAME ERROR]");
            return false;
        
        case UartFrameConstants::UART_FRAME_ERROR_TRAILER_MISMATCH:
            PLAT_LOG_D(__FMT_STR__, "[TRAILER MISMATCH FRAME ERROR]");
            return false;
        case UartFrameConstants::UART_FRAME_ERROR_NONCE_MISMATCH:
            PLAT_LOG_D(__FMT_STR__, "[NONCE MISMATCH FRAME ERROR]");
            return false;
        case UartFrameConstants::UART_FRAME_INVALID_AAD_LEN:
            PLAT_LOG_D(__FMT_STR__, "[INVALID AAD LENGTH FRAME ERROR]");
            return false;
        case UartFrameConstants::UART_FRAME_INVALID_SECRET_LEN:
            PLAT_LOG_D(__FMT_STR__, "[INVALID SECRET LENGTH FRAME ERROR]");
            return false;
        case UartFrameConstants::UART_FRAME_DECRYPTION_FAILED:
            PLAT_LOG_D(__FMT_STR__, "[DECRYPTION FAILED FRAME ERROR]");
            return false;
        default:
            PLAT_LOG_D("[HEADER_1 FAILED] actual: %d, expect: %d",
                      byteFrame, 
                      UartFrameConstants::UART_FRAME_HEADER_1);
            return false;
    }
}

bool UartFrame::isSecondHeaderByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_HEADER_2)
    {
        // PLAT_LOG_D("[HEADER_2 PASSED] %d", byteFrame);
        return true;
    }
    PLAT_LOG_D("[HEADER_2 FAILED] actual: %d, expect: %d", byteFrame, UartFrameConstants::UART_FRAME_HEADER_2);
    return false;
}

bool UartFrame::isDataLengthFirstByteValid(uint8_t byteFrame)
{
    if (byteFrame <= UART_FRAME_MAX_DATA_SIZE)
    {
        // PLAT_LOG_D("[DATA_LENGHT_1 PASSED] %d", byteFrame);
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
        // PLAT_LOG_D("[DATA_LENGHT_2 PASSED] %d", byteFrame);
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
        // PLAT_LOG_D("[TRAILER_1 PASSED] %d", byteFrame);
        return true;
    }
    PLAT_LOG_D("[TRAILER_1 FAILED] actual: %d, expect: %d", byteFrame, UartFrameConstants::UART_FRAME_TRAILER_1);
    return false;
}

bool UartFrame::isSecondTrailerByteValid(uint8_t byteFrame)
{
    if (byteFrame == UartFrameConstants::UART_FRAME_TRAILER_2)
    {
        // PLAT_LOG_D("[TRAILER_2 PASSED] %d", byteFrame);
        return true;
    }
    PLAT_LOG_D("[TRAILER_2 FAILED] actual: %d, expect: %d", byteFrame, UartFrameConstants::UART_FRAME_TRAILER_2);
    return false;
}

bool UartFrame::isCrcFirstByteValid(uint8_t byteFrame)
{
    if (byteFrame <= BYTE_MAX)
    {
        // PLAT_LOG_D("[BYTE PASSED] %d", byteFrame);
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
        // PLAT_LOG_D("[BYTE PASSED] %d", byteFrame);
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

void UartFrame::collectData(uint8_t byteFrame)
{
    if (m_frameBuffer.size() < UART_FRAME_MAX_DATA_SIZE)
    {
        // PLAT_LOG_D("[DATA] %02X", byteFrame);
        m_frameBuffer.push_back(byteFrame);
    }
    else
    {
        PLAT_ASSERT((m_frameBuffer.size() < UART_FRAME_MAX_DATA_SIZE),
                    "[BUFFER OVERFLOW for Data] Max data size reached: %d",
                    UART_FRAME_MAX_DATA_SIZE);
        m_parserNextState = UartParserState::FRAME_ERROR;
    }
}

void UartFrame::collectAuthTag(uint8_t byteFrame)
{
    if (m_authTag.size() < UART_FRAME_MAX_DATA_SIZE)
    {
        // PLAT_LOG_D("[DATA] %d", byteFrame);
        m_authTag.push_back(byteFrame);
    }
    else
    {
        PLAT_ASSERT((m_authTag.size() < UART_FRAME_MAX_DATA_SIZE),
                    "[BUFFER OVERFLOW for Auth Tag] Max data size reached: %d",
                    UART_FRAME_MAX_DATA_SIZE);
        m_parserNextState = UartParserState::FRAME_ERROR;
    }
}

void UartFrame::collectDeviceID(uint8_t byteFrame)
{
    if(m_deviceID.size() < Device::BYTE_LENGTH)
    {
        // PLAT_LOG_D("[ID] %d", byteFrame);
        m_deviceID.push_back(byteFrame);
    }
    else
    {
        PLAT_ASSERT((m_deviceID.size() < Device::BYTE_LENGTH),
                    "[BUFFER OVERFLOW for ID] Max data size reached: %d",
                    Device::BYTE_LENGTH);
        m_parserNextState = UartParserState::FRAME_ERROR;
    }
}

void UartFrame::collectNonce(uint8_t byteFrame)
{
    if(m_nonceReceive.size() < NONCE_SIZE)
    {
        // PLAT_LOG_D("[NONCE] %02X", byteFrame);
        m_nonceReceive.push_back(byteFrame);
    }
    else
    {
        PLAT_ASSERT((m_nonceReceive.size() < NONCE_SIZE),
                    "[BUFFER OVERFLOW for NONCE] Max data size reached: %d",
                    NONCE_SIZE);
        m_parserNextState = UartParserState::FRAME_ERROR;
    }
}

bool UartFrame::parseFrame(std::vector<uint8_t> byteBuffer)
{
    for (uint8_t i : byteBuffer)
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
    if (!m_isParsingActive)
        return;
    uint32_t currentTime = Platform::GetCurrentTimeMs();
    if ((currentTime - m_lastByteTimestamp) >= UartTimer::UART_FRAME_TIMEOUT_MS)
    {
        PLAT_LOG_D(__FMT_STR__, "Timeout! Resetting parser.");
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

void UartFrame::beginUartCommunication()
{
    m_uart->begin(Serial::BAUD_RATE, SERIAL_8N1, Serial::TX_PIN, Serial::RX_PIN);
    PLAT_LOG_D(__FMT_STR__, "UART initialized");
}

bool UartFrame::update()
{
    PLAT_ASSERT_NULL(m_uart, __FMT_STR__, "UART instance is null");
    PLAT_LOG_D(__FMT_STR__, "-- Receiving data from STM32");
    resetFrameBuffer();

    // Wait for data with a timeout, avoid condition where data from STM32 have not arrived
    auto startTime = std::chrono::high_resolution_clock::now();
    const int MAX_WAIT_MS = 100; 
    while (m_uart->available() == 0)
    {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - startTime).count();
        if (elapsed > MAX_WAIT_MS)
        {
            PLAT_LOG_D(__FMT_STR__, "-- Timeout: No data received from STM32");
            resetStateMachine();
            return false; 
        }
    }

    while (m_uart->available())
    {
        uint8_t byte = m_uart->read();
        m_frameReceiveBuffer.push_back(byte);
        // PLAT_LOG_D("[==FRAME STATE==] Byte received: %d", byte);
    }

    // PLAT_LOG_D("[==FRAME STATE==] Frame buffer size: %d", m_frameReceiveBuffer.size());
    auto startParseTime = std::chrono::high_resolution_clock::now();
    bool isParseProcessComplete = parseFrame(m_frameReceiveBuffer);
    auto endParseTime = std::chrono::high_resolution_clock::now();
    double elapsedParseTime = std::chrono::duration<double, std::milli>(endParseTime - startParseTime).count();
    PLAT_LOG_D("-- Frame parsing completed in %.2f ms", elapsedParseTime);

    resetStateMachine();
    return isParseProcessComplete;
}

bool UartFrame::isParsingComplete()
{
    return m_isParsingComplete;
}

void UartFrame::constructFrameForTransmittingKeySTM32(const STM32FrameParams& params)
{

    try {
        m_uartFrameSTM32->str_header[0] = UartFrameConstants::UART_FRAME_HEADER_1;
        m_uartFrameSTM32->str_header[1] = UartFrameConstants::UART_FRAME_HEADER_2;
        m_uartFrameSTM32->str_packetType = UARTCommand::STM_RECEIVE_KEY;
        std::copy(IdentifierIDSTM, IdentifierIDSTM + IDENTIFIER_ID_STM_SIZE, m_uartFrameSTM32->str_identifierId);
        std::copy(params.secretKey.begin(), params.secretKey.begin() + SECRET_KEY_SIZE, m_uartFrameSTM32->str_secretKey);
        std::copy(params.nonce.begin(), params.nonce.begin() + NONCE_SIZE, m_uartFrameSTM32->str_nonce);
        std::copy(params.aad.begin(), params.aad.begin() + AAD_MAX_SIZE, m_uartFrameSTM32->str_add);
        std::copy(params.authTag.begin(), params.authTag.begin() + AUTH_TAG_SIZE, m_uartFrameSTM32->str_authTag);
        m_uartFrameSTM32->str_addLength[0] = params.aad.size() & 0xFF; // LSB
        m_uartFrameSTM32->str_addLength[1] = (params.aad.size() >> 8) & 0xFF; // MSB
        m_uartFrameSTM32->str_secretKeyLength[0] = params.secretKey.size() & 0xFF; // LSB
        m_uartFrameSTM32->str_secretKeyLength[1] = (params.secretKey.size() >> 8) & 0xFF; // MSB
        m_uartFrameSTM32->str_eof[0] = UartFrameConstants::UART_FRAME_TRAILER_1;
        m_uartFrameSTM32->str_eof[1] = UartFrameConstants::UART_FRAME_TRAILER_2;
        // PLAT_LOG_D(__FMT_STR__, "-- Constructed frame for transmitting key to STM32");
    }
    catch (const std::exception& e) {
        PLAT_LOG_D("[ERROR] Exception in constructFrameForTransmittingKeySTM32: %s", e.what());
    }
}

void UartFrame::constructFrameForTransmittingTriggerSignal(std::vector<uint8_t> associatedData)
{
    try {
        m_uartFrameSTM32Trigger->str_header[0] = UartFrameConstants::UART_FRAME_HEADER_1;
        m_uartFrameSTM32Trigger->str_header[1] = UartFrameConstants::UART_FRAME_HEADER_2;
        std::copy(IdentifierIDSTM, IdentifierIDSTM + IDENTIFIER_ID_STM_SIZE, m_uartFrameSTM32Trigger->str_identifierId);
        m_uartFrameSTM32Trigger->str_triggerSignal = UARTCommand::SIGNAL;
        std::copy(associatedData.begin(), associatedData.begin() + AAD_MAX_SIZE, m_uartFrameSTM32Trigger->str_add);
        m_uartFrameSTM32Trigger->str_addLength[0] = associatedData.size() & 0xFF; // LSB
        m_uartFrameSTM32Trigger->str_addLength[1] = (associatedData.size() >> 8) & 0xFF; // MSB
        m_uartFrameSTM32Trigger->str_eof[0] = UartFrameConstants::UART_FRAME_TRAILER_1;
        m_uartFrameSTM32Trigger->str_eof[1] = UartFrameConstants::UART_FRAME_TRAILER_2;
        // PLAT_LOG_D(__FMT_STR__, "-- Constructed frame for transmitting trigger signal to STM32");
    }
    catch (const std::exception& e) {
        PLAT_LOG_D("[ERROR] Exception in constructFrameForTransmittingKeySTM32: %s", e.what());
    }
}

std::vector<uint8_t> UartFrame::getNonce()
{
    return m_nonceReceive;
}

std::vector<uint8_t> UartFrame::getAuthTag()
{
    return m_authTag;
}

int UartFrame::getFrameBufferSize()
{
    return m_frameBuffer.size();
}

void UartFrame::resetStateMachine()
{
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
