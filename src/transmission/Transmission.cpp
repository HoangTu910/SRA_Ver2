#include "transmission/Transmissions.hpp"

Transmissions::Transmissions() : 
        m_uart(Transmission::UartFrame::UartFrame::create()),
        m_ascon128a(Cryptography::Ascon128a::create())
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
                m_data = m_uart->getFrameBuffer().data();
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
        case TransmissionState::PROCESS_ENCRYPTION:
            /**
             * @brief Set the key and nonce for encryption
             */
            // setupDataForEncryption(); // will be implemented later
            // m_ascon128a->encrypt();
            m_transmissionNextState = TransmissionState::SEND_DATA_TO_SERVER;
            PLAT_LOG_D(__FMT_STR__, "STEP 3 - ENCRYPTION COMPLETE");
            break;
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
