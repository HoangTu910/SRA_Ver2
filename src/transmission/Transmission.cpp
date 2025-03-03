#include "transmission/Transmissions.hpp"

Transmissions::Transmissions() : 
    m_uart(Transmission::UartFrame::UartFrame::create()),
    m_ascon128a(Cryptography::Ascon128a::create()),
    m_server(Transmission::ServerFrame::ServerFrame::create()),
    m_mqtt(MQTT::create(MQTTHelper::MQTT_SERVER, 
                        MQTTHelper::MQTT_PORT, 
                        MQTTHelper::MQTT_DEVICE_ID, 
                        MQTTHelper::MQTT_DATA_TOPIC, 
                        MQTTHelper::MQTT_PUBLIC_KEY_TOPIC,
                        MQTTHelper::MQTT_USER,
                        MQTTHelper::MQTT_PASSWORD,
                        MQTTHelper::MQTT_PUBLIC_KEY_RECEIVE_TOPIC)),
    m_dataLength(0)
{
    // Initialize empty vector instead of nullptr
    m_data = std::vector<unsigned char>();
    
    // Setup MQTT server if connection pointer is valid
    if (m_mqtt) {
        m_mqtt->setupServer();
    }
    
    // Initialize transmission state
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

bool Transmissions::startTransmissionProcess()
{
    PLAT_ASSERT_NULL(m_uart, __FMT_STR__, "uartFrame is uninitilized");
    PLAT_ASSERT_NULL(m_ascon128a, __FMT_STR__, "ascon128a is uninitilized");
    switch(m_transmissionNextState)
    {
        case TransmissionState::PROCESS_FRAME_PARSING:{
            auto startTime = std::chrono::high_resolution_clock::now();
            bool isUpdateAndParsingComplete = m_uart->update();
            if(isUpdateAndParsingComplete)
            {
                m_data = m_uart->getFrameBuffer();  // Simply assign the frame buffer vector
                m_dataLength = m_uart->getFrameBuffer().size();
                m_transmissionNextState = TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE;
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_frameProcessTime += elapsedTime;
                PLAT_LOG_D("[1/5] Frame parsing completed in %.2f ms", elapsedTime);
                m_isFrameParsing = true;
            }
            else{
                m_isFrameParsing = false;
                m_transmissionNextState = TransmissionState::TRANSMISSION_ERROR;
                PLAT_LOG_D(__FMT_STR__, "[1/5] - OH FUKK what did you send? -_-");
            }
            m_uart->resetFrameBuffer();
            break;
        }
        case TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE:{
            if(m_server->getSequenceNumber() == ServerFrameConstants::SERVER_FRAME_SEQUENCE_NUMBER){
                PLAT_LOG_D(__FMT_STR__, "-- Key Expired! Renewing...");
                auto startTime = std::chrono::high_resolution_clock::now();
                while(m_server->getHandshakeState() != HandshakeState::HANDSHAKE_COMPLETE)
                {
                    m_server->performHandshake(m_mqtt);
                }
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                PLAT_LOG_D("[2/5] Handshake for key exchanging completed in %.2f ms", elapsedTime);
                m_handshakeProcessTime += elapsedTime;
                m_server->resetHandshakeState();
            }
            else {
                auto startTime = std::chrono::high_resolution_clock::now();
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_handshakeProcessTime += elapsedTime;
                PLAT_LOG_D("[2/5] Key Verified! Completed in %.2f ms", elapsedTime);
            }
            
            m_transmissionNextState = TransmissionState::PROCESS_ENCRYPTION;
            break;
        }
        case TransmissionState::PROCESS_ENCRYPTION:{
            auto startTime = std::chrono::high_resolution_clock::now();
            m_ascon128a->setNonce();
            m_ascon128a->setPlainText(m_data);
            m_ascon128a->setKey(m_server->getSecretKeyComputed()); 
            m_ascon128a->encrypt();
            // m_ascon128a->decrypt(); //Decrypt here just for verifying if the data is correct
            auto endTime = std::chrono::high_resolution_clock::now();
            double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            m_encryptionProcessTime += elapsedTime;
            PLAT_LOG_D("[3/5] Encryption (Ascon-128a) completed in %.2f ms", elapsedTime);
            m_transmissionNextState = TransmissionState::SEND_DATA_TO_SERVER;
            break;
        }
        case TransmissionState::SEND_DATA_TO_SERVER:
        {
            // Send the data to server
            auto startTime = std::chrono::high_resolution_clock::now();
            m_server->sendDataFrameToServer(m_mqtt, 
                                            m_ascon128a->getNonce(),
                                            m_ascon128a->getCipherTextLength(),
                                            m_ascon128a->getCipherText());
            
            auto endTime = std::chrono::high_resolution_clock::now();
            double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            m_sendDataProcessTime += elapsedTime;
            PLAT_LOG_D("[4/5] Send data to server completed in %.2f ms", elapsedTime);
            m_transmissionNextState = TransmissionState::WAIT_FOR_ACK_PACKAGE;
            break;
        }
        case TransmissionState::WAIT_FOR_ACK_PACKAGE:
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            if(m_server->isAckFromServerArrived(m_mqtt)){
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_ackResponseTime += elapsedTime;
                m_transmissionNextState = TransmissionState::TRANSMISSION_COMPLETE;
                PLAT_LOG_D("[5/5] Received ACK package from server in %.2f ms", elapsedTime);
            }
            else if(m_mqtt->m_mqttIsTimeout){
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_transmissionNextState = TransmissionState::TRANSMISSION_ERROR;
                PLAT_LOG_D("[5/5] OH FUKK where is ACK? -_- (Timeout after %.2f ms)", elapsedTime);
            }
            else{
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_transmissionNextState = TransmissionState::TRANSMISSION_ERROR;
                PLAT_LOG_D("[5/5] OH FUKK what did you send? -_- (Failed after %.2f ms)", elapsedTime);
            }
            break;
        }
        case TransmissionState::TRANSMISSION_ERROR:
        {
            PLAT_LOG_D(__FMT_STR__, "[DAMN] Transmission error");
            handleTransmissionError();
            return false;
            break;
        }
        case TransmissionState::TRANSMISSION_COMPLETE:
        {
            PLAT_LOG_D(__FMT_STR__, "[NICE] Transmission completed!");
            resetTransmissionState();
            return true;
            break;
        }
    }
    return false;
}

void Transmissions::loopMqtt()
{
    m_mqtt->connect();
}
