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
    m_transmissionNextState = TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE;
    m_transmissionFinalState = TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE;
    m_dataLength = 0;
}

bool Transmissions::startTransmissionProcess()
{
    PLAT_ASSERT_NULL(m_uart, __FMT_STR__, "uartFrame is uninitilized");
    PLAT_ASSERT_NULL(m_ascon128a, __FMT_STR__, "ascon128a is uninitilized");
    switch(m_transmissionNextState)
    {
        case TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE:{
            PLAT_LOG_D(__FMT_STR__, "[1/4] Checking key...");
            if(m_server->getSequenceNumber() == ServerFrameConstants::RESET_SEQUENCE || 
                m_server->getSequenceNumber() == ServerFrameConstants::INITIAL_SEQUENCE){
                m_isHandshake = true;    
                PLAT_LOG_D(__FMT_STR__, "-- Key Expired! Renewing...");

                auto startTime = std::chrono::high_resolution_clock::now();
                while(m_server->getHandshakeState() != HandshakeState::HANDSHAKE_COMPLETE)
                {
                    m_server->performHandshake(m_mqtt);
                }
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                PLAT_LOG_D("-- Handshake for key exchanging completed in %.2f ms", elapsedTime);

                // Construct frame for transmitting key to STM32
                PLAT_LOG_D(__FMT_STR__, "-- Construct frame key to STM32...");
                m_uart->constructFrameForTransmittingKeySTM32(m_server->getSecretKeyComputed().data());
                PLAT_LOG_D(__FMT_STR__, "-- Transmitting key to STM32...");
                m_uart->transmitData(*m_uart->getUartFrameSTM32()); // pass * to get data

                m_handshakeProcessTime += elapsedTime;
                m_server->resetHandshakeState();
            }
            else {
                PLAT_LOG_D(__FMT_STR__, "-- Key is still valid");
                auto startTime = std::chrono::high_resolution_clock::now();
                auto endTime = std::chrono::high_resolution_clock::now();
                m_uart->constructFrameForTransmittingTriggerSignal();
                PLAT_LOG_D(__FMT_STR__, "-- Transmiting trigger signal to STM32");
                m_uart->transmitData(*m_uart->getUartFrameSTM32Trigger()); // pass * to get data

                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_handshakeProcessTime += elapsedTime;
            }
            m_transmissionNextState = TransmissionState::WAIT_STATE_FOR_SERVER_PROCESSING_KEY;
            break;
        }
        case TransmissionState::WAIT_STATE_FOR_SERVER_PROCESSING_KEY:{
            PLAT_LOG_D(__FMT_STR__, "-- Waiting for server to process key...");
            __WAIT_STATE_FOR_SERVER_PROCESSING_KEY__;
            m_transmissionNextState = TransmissionState::PROCESS_FRAME_PARSING;
            break;
        }
        case TransmissionState::PROCESS_FRAME_PARSING:{
            PLAT_LOG_D(__FMT_STR__, "[2/4] Receive and process frame...");
            auto startTime = std::chrono::high_resolution_clock::now();
            bool isUpdateAndParsingComplete = m_uart->update();
            if(isUpdateAndParsingComplete)
            {
                m_data = m_uart->getFrameBuffer();  
                m_dataLength = m_uart->getFrameBufferSize();
                m_nonce = m_uart->getNonce();
                m_transmissionNextState = TransmissionState::SEND_DATA_TO_SERVER;
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_frameProcessTime += elapsedTime;
                PLAT_LOG_D("-- Frame parsing completed in %.2f ms", elapsedTime);
                m_isFrameParsing = true;
            }
            else{
                m_isFrameParsing = false;
                m_transmissionNextState = TransmissionState::TRANSMISSION_ERROR;
                PLAT_LOG_D(__FMT_STR__, "-- OH FUKK what did you send? -_-");
            }
            m_uart->resetFrameBuffer();
            break;
        }
        // case TransmissionState::PROCESS_ENCRYPTION:{
        //     PLAT_LOG_D(__FMT_STR__, "[3/5] Skipping encryption step...");
        //     auto startTime = std::chrono::high_resolution_clock::now();
        //     // m_ascon128a->setNonce();
        //     // m_ascon128a->setPlainText(m_data);
        //     // m_ascon128a->setKey(m_server->getSecretKeyComputed()); 
        //     // m_ascon128a->encrypt();
        //     // m_ascon128a->decrypt(); //Decrypt here just for verifying if the data is correct
        //     auto endTime = std::chrono::high_resolution_clock::now();
        //     double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        //     m_encryptionProcessTime += elapsedTime;
        //     PLAT_LOG_D("-- Encryption (Ascon-128a) completed in %.2f ms", elapsedTime);
        //     m_transmissionNextState = TransmissionState::SEND_DATA_TO_SERVER;
        //     break;
        // }
        case TransmissionState::SEND_DATA_TO_SERVER:
        {
            // Send the data to server
            PLAT_LOG_D(__FMT_STR__, "[3/4] Construct and send data to server...");
            auto startTime = std::chrono::high_resolution_clock::now();
            m_server->sendDataFrameToServer(m_mqtt, 
                                            m_nonce,
                                            m_dataLength,
                                            m_data);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            m_sendDataProcessTime += elapsedTime;
            PLAT_LOG_D("-- Send data to server completed in %.2f ms", elapsedTime);
            m_transmissionNextState = TransmissionState::WAIT_FOR_ACK_PACKAGE;
            break;
        }
        case TransmissionState::WAIT_FOR_ACK_PACKAGE:
        {
            PLAT_LOG_D(__FMT_STR__, "[4/4] Waiting for ACK package...");
            auto startTime = std::chrono::high_resolution_clock::now();
            if(m_server->isAckFromServerArrived(m_mqtt)){
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_ackResponseTime += elapsedTime;
                m_transmissionNextState = TransmissionState::TRANSMISSION_COMPLETE;
                PLAT_LOG_D("-- Received ACK package from server in %.2f ms", elapsedTime);
            }
            else if(m_mqtt->m_mqttIsTimeout){
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_server->resetSequenceNumber();
                m_transmissionNextState = TransmissionState::TRANSMISSION_ERROR;
                PLAT_LOG_D("-- OH FUKK where is ACK? -_- (Timeout after %.2f ms)", elapsedTime);
            }
            else{
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                m_server->resetSequenceNumber();
                m_transmissionNextState = TransmissionState::TRANSMISSION_ERROR;
                PLAT_LOG_D("-- OH FUKK what did you send? -_- (Failed after %.2f ms)", elapsedTime);
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
