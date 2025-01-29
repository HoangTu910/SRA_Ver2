#include "transmission/Transmissions.hpp"

Transmissions::Transmissions() : 
        m_uart(Transmission::UartFrame::UartFrame::create()),
        m_ascon128a(Cryptography::Ascon128a::create()),
        m_data(nullptr),
        m_dataLength(0)
{
    resetTransmissionState();
}

Transmissions::~Transmissions()
{

}

std::shared_ptr<Transmissions> Transmissions::create()
{
    return std::make_shared<Transmissions>();
}

void Transmissions::handleTransmissionError()
{
    resetTransmissionState();
}

void Transmissions::resetTransmissionState()
{
    m_transmissionNextState = TransmissionState::PROCESS_FRAME_PARSING;
    m_transmissionFinalState = TransmissionState::PROCESS_FRAME_PARSING;
    m_dataLength = 0;
}

void Transmissions::startTransmissionProcess()
{
    PLAT_ASSERT_NULL(m_uart, __FMT_STR__, "uartFrame is uninitilized");
    PLAT_ASSERT_NULL(m_ascon128a, __FMT_STR__, "ascon128a is uninitilized");
    switch(m_transmissionNextState)
    {
        case TransmissionState::PROCESS_FRAME_PARSING:{
            bool isUpdateAndParsingComplete = m_uart->update();
            if(isUpdateAndParsingComplete)
            {
                std::vector<uint8_t> frameBuffer = m_uart->getFrameBuffer();  
                if (m_data) {
                    delete[] m_data; 
                    m_data = nullptr;
                }
                m_data = new unsigned char[frameBuffer.size()];  
                memcpy(m_data, frameBuffer.data(), frameBuffer.size()); 
                m_dataLength = m_uart->getFrameBuffer().size();
                m_transmissionNextState = TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE;
                PLAT_LOG_D(__FMT_STR__, "STEP 1 - FRAME PARSING COMPLETE");
            }
            else{
                m_transmissionNextState = TransmissionState::TRANSMISSION_ERROR;
                PLAT_LOG_D(__FMT_STR__, "STEP 1 - OH FUK FRAME NOT GUD");
            }
            m_uart->resetFrameBuffer();
            break;
        }
        case TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE:
            // Process handshake using EDHC to get key for encryption
            // ascon128a->processHandshake();
            PLAT_LOG_D(__FMT_STR__, "STEP 2 - HANDSHAKE AND KEY EXCHANGE COMPLETE");
            m_transmissionNextState = TransmissionState::PROCESS_ENCRYPTION;
            break;
        case TransmissionState::PROCESS_ENCRYPTION:{\
            m_ascon128a->setPlainText(m_data, m_dataLength);
            unsigned char fixedKey[ASCON_KEY_SIZE] = {
                0x01, 0x02, 0x03, 0x04,
                0x05, 0x06, 0x07, 0x08,
                0x09, 0x0A, 0x0B, 0x0C,
                0x0D, 0x0E, 0x0F, 0x10
            };
            for(int i = 0; i < m_dataLength; i++)
            {
                PLAT_LOG_D("Data[%d]: %d", i, m_data[i]);
            }
            m_ascon128a->setKey(fixedKey); // will be replaced with key from handshake
            m_ascon128a->encrypt();
            m_ascon128a->decrypt(); //Decrypt here just for verifying if the data is correct
            m_transmissionNextState = TransmissionState::SEND_DATA_TO_SERVER;
            PLAT_LOG_D(__FMT_STR__, "STEP 3 - ENCRYPTION COMPLETE");
            break;
        }
        case TransmissionState::SEND_DATA_TO_SERVER:
            // Send the data to server
            // sendDataToServer(); // will be implemented later
            PLAT_LOG_D(__FMT_STR__, "STEP 4 - SENDING DATA TO SERVER COMPLETE");
            m_transmissionNextState = TransmissionState::TRANSMISSION_COMPLETE;
            break;
        case TransmissionState::TRANSMISSION_ERROR:
            PLAT_LOG_D(__FMT_STR__, "Transmission error");
            handleTransmissionError();
            break;
        case TransmissionState::TRANSMISSION_COMPLETE:
            PLAT_LOG_D(__FMT_STR__, "Transmission complete");
            resetTransmissionState();
            break;
    }
}
