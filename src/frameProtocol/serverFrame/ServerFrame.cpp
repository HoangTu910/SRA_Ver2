#include "frameProtocol/serverFrame/ServerFrame.hpp"

Transmission::ServerFrame::ServerFrame::ServerFrame() : 
    m_secretKeyComputed(ECC_PUB_KEY_SIZE),
    m_ascon128a(Cryptography::Ascon128a::create())
{
    m_handshakeFrame = std::make_shared<Handshake::HandshakeFrameData>();
    m_serverDataFrame = std::make_shared<DataFrame::ServerFrameData>();
    resetHandshakeState();
}

Transmission::ServerFrame::ServerFrame::~ServerFrame()
{
}

std::shared_ptr<Transmission::ServerFrame::ServerFrame> Transmission::ServerFrame::ServerFrame::create()
{
    return std::make_shared<ServerFrame>();
}

void Transmission::ServerFrame::ServerFrame::performHandshake(std::shared_ptr<MQTT> mqtt)
{
    if (!mqtt) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: null MQTT pointer");
        return;
    }
    try {
        switch(m_handshakeNextState)
        {
            case HandshakeState::GENERATE_PUBLIC_KEY:
            {
                try {
                    static int initialized = 0;
                    if (!initialized)
                    {
                        prng_init((0xbad ^ 0xc0ffee ^ 42) | 0xcafebabe | 666);
                        initialized = 1;
                    }
                    
                    if (ECC_PRV_KEY_SIZE <= 0) {
                        PLAT_LOG_D(__FMT_STR__, "-- Invalid ECC_PRV_KEY_SIZE");
                        return;
                    }
                    
                    std::vector<uint8_t> tempPrivKey(ECC_PRV_KEY_SIZE);
                    for (int i = 0; i < ECC_PRV_KEY_SIZE; ++i)
                    {
                        tempPrivKey[i] = prng_next();
                    }
                    
                    std::copy(tempPrivKey.begin(), tempPrivKey.end(), ECDH::devicePrivateKey);
                    
                    if (!ECDH::ecdh_generate_keys(ECDH::devicePublicKey, ECDH::devicePrivateKey)) {
                        PLAT_LOG_D(__FMT_STR__, "-- Failed to generate ECDH keys");
                        return;
                    }
                    
                    m_handshakeNextState = HandshakeState::CONSTRUCT_PUBLIC_KEY_FRAME;
                }
                catch (const std::exception& e) {
                    PLAT_LOG_D("Exception in key generation: %s", e.what());
                    return;
                }
                break;
            }
            case HandshakeState::CONSTRUCT_PUBLIC_KEY_FRAME:
            {
                std::vector<unsigned char> associatedData = {0x48, 0x45, 0x4C, 0x4C, 0x4F};
                std::vector<unsigned char> smallPlaintext = {};
                auto startTime_ = std::chrono::high_resolution_clock::now();
                m_ascon128a->setAssociatedData(associatedData);
                m_ascon128a->setNonce();
                m_ascon128a->setPlainText(smallPlaintext); 
                m_ascon128a->setKey(m_ascon128a->getPresharedSecretKey());
                
                // auto startTime = std::chrono::high_resolution_clock::now();
                m_ascon128a->encrypt();
                // auto endTime = std::chrono::high_resolution_clock::now();
                // double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                // PLAT_LOG_D("-- Auth tag generated (Ascon-128a) in %.2f ms", elapsedTime);
                
                m_handshakeFrame->s_preamble = SERVER_FRAME_PREAMBLE;
                m_handshakeFrame->s_identifierId = SERVER_FRAME_IDENTIFIER_ID;
                m_handshakeFrame->s_packetType = SERVER_FRAME_PACKET_HANDSHAKE_TYPE;
                m_handshakeFrame->s_publicKeyLength = ECC_PUB_KEY_SIZE;
                auto nonce = m_ascon128a->getNonce();
                if (!nonce.empty()) {
                    std::copy_n(nonce.begin(), std::min(nonce.size(), (size_t)NONCE_SIZE), m_handshakeFrame->s_nonce);
                }

                auto authTag = m_ascon128a->getAuthTagFromCipherText();
                if (!authTag.empty()) {
                    std::copy_n(authTag.begin(), std::min(authTag.size(), (size_t)AUTH_TAG_SIZE), m_handshakeFrame->s_authTag);
                }
                m_handshakeFrame->s_endMarker = SERVER_FRAME_END_MAKER;
                if (ECC_PUB_KEY_SIZE > 0) {
                    std::copy(ECDH::devicePublicKey, ECDH::devicePublicKey + ECC_PUB_KEY_SIZE, m_handshakeFrame->s_publicKey);
                }
                auto endTime_ = std::chrono::high_resolution_clock::now();
                double elapsedTime_ = std::chrono::duration<double, std::milli>(endTime_ - startTime_).count();

                PLAT_LOG_D("-- Contructed public key frame in %.2f ms", elapsedTime_);
                m_handshakeNextState = HandshakeState::SEND_PUBLIC_KEY_FRAME;
                break;
            }
            case HandshakeState::SEND_PUBLIC_KEY_FRAME:
            {
                if (!m_handshakeFrame) {
                    PLAT_LOG_D(__FMT_STR__, "-- Error: m_handshakeFrame is null");
                    m_handshakeNextState = HandshakeState::GENERATE_PUBLIC_KEY; 
                    break;
                }
                if (!mqtt) {
                    PLAT_LOG_D(__FMT_STR__, "-- Error: mqtt is null");
                    m_handshakeNextState = HandshakeState::GENERATE_PUBLIC_KEY;
                    break;
                }
                mqtt->publishData(m_handshakeFrame.get(), sizeof(Handshake::HandshakeFrameData));
                PLAT_LOG_D(__FMT_STR__, "-- Sent public key frame to server");
                m_handshakeNextState = HandshakeState::WAIT_FOR_PUBLIC_FROM_SERVER;
                break;
            }
            case HandshakeState::WAIT_FOR_PUBLIC_FROM_SERVER:
            {
                /**
                 * @brief Need to optimize and reconstruct the code for further maintain
                 * @brief Need timeout to handle error
                 */
                unsigned long startTime = millis(); 
                const unsigned long timeout = 5000; 
                while (!mqtt->m_mqttIsMessageArrived)
                {
                    mqtt->connect();
                    if (millis() - startTime >= timeout) {
                        PLAT_LOG_D(__FMT_STR__, "-- Timeout waiting for server public key");
                        m_handshakeNextState = HandshakeState::HANDSHAKE_COMPLETE;
                        return;
                    }
                    delay(10);
                }

                if (mqtt->m_mqttIsMessageArrived) {
                    mqtt->m_mqttIsMessageArrived = false; // Reset flag
                    m_handshakeNextState = HandshakeState::COMPUTE_SHARED_SECRET;
                    PLAT_LOG_D(__FMT_STR__, "-- Received public key from server");
                }
                break;
            }
            case HandshakeState::COMPUTE_SHARED_SECRET:
            {
                try {
                    if (!mqtt || !mqtt->m_mqttCallBackDataReceive.data() || mqtt->m_mqttCallBackDataReceive.empty()) {
                        PLAT_LOG_D(__FMT_STR__, "-- Invalid input data for shared secret computation");
                        m_handshakeNextState = HandshakeState::GENERATE_PUBLIC_KEY;
                        return;
                    }

                    if (m_secretKeyComputed.size() != ECC_PUB_KEY_SIZE) {
                        m_secretKeyComputed.resize(ECC_PUB_KEY_SIZE);
                    }

                    if (!ECDH::ecdh_shared_secret(ECDH::devicePrivateKey, 
                                                mqtt->m_mqttCallBackDataReceive.data(), 
                                                m_secretKeyComputed.data())) {
                        PLAT_LOG_D(__FMT_STR__, "-- Failed to compute shared secret");
                        m_handshakeNextState = HandshakeState::GENERATE_PUBLIC_KEY;
                        return;
                    }

                    PLAT_LOG_ED(__FMT_STR__, "-- Shared secret computed successfully ");
                    for(int i = 0; i < 16; i++) {
                        PLAT_LOG_ED("%02X ", m_secretKeyComputed[i]);
                    }
                    PLAT_LOG_ED("\n");
                    m_handshakeNextState = HandshakeState::HANDSHAKE_COMPLETE;
                }
                catch (const std::exception& e) {
                    PLAT_LOG_D("Exception in shared secret computation: %s", e.what());
                    m_handshakeNextState = HandshakeState::GENERATE_PUBLIC_KEY;
                }
                break;
            }
        }
    } catch (const std::exception& e) {
        PLAT_LOG_D("Exception in performHandshake: %s", e.what());
        m_handshakeNextState = HandshakeState::GENERATE_PUBLIC_KEY;
    }
}

void Transmission::ServerFrame::ServerFrame::resetHandshakeState()
{
    m_handshakeNextState = HandshakeState::GENERATE_PUBLIC_KEY;
}

HandshakeState Transmission::ServerFrame::ServerFrame::getHandshakeState()
{
    return m_handshakeNextState;
}

void Transmission::ServerFrame::ServerFrame::constructServerDataFrame(const std::vector<unsigned char>& nonce, 
                                                                    unsigned long long cipherTextLength,
                                                                    const std::vector<unsigned char>& cipherText,
                                                                    const std::vector<unsigned char>& authTag)
{
    // Set frame header fields
    PLAT_LOG_D(__FMT_STR__, "-- Constructing server data frame");
    auto startTime = std::chrono::steady_clock::now();
    m_serverDataFrame->s_preamble = SERVER_FRAME_PREAMBLE;
    m_serverDataFrame->s_identifierId = SERVER_FRAME_IDENTIFIER_ID;  
    m_serverDataFrame->s_packetType = SERVER_FRAME_PACKET_DATA_TYPE;
    // currentSequenceNumber();
    if(m_serverDataFrame->s_sequenceNumber >= ServerFrameConstants::SERVER_FRAME_SEQUENCE_NUMBER) 
    {
        m_serverDataFrame->s_sequenceNumber = ServerFrameConstants::RESET_SEQUENCE;
    }
    else
    {
        m_serverDataFrame->s_sequenceNumber++;
    }
    PLAT_LOG_D("-- Sequence number: %d", m_serverDataFrame->s_sequenceNumber);

    // Copy nonce
    if(nonce.size() >= NONCE_SIZE) {
        std::copy(nonce.begin(), nonce.begin() + NONCE_SIZE, m_serverDataFrame->s_nonce);
    }

    m_serverDataFrame->s_payloadLength = cipherTextLength;
    if(cipherTextLength <= sizeof(m_serverDataFrame->s_encryptedPayload)) {
        std::copy(cipherText.begin(), 
                 cipherText.begin() + cipherTextLength,
                 m_serverDataFrame->s_encryptedPayload);
    }

    if(authTag.size() <= AUTH_TAG_SIZE) {
        std::copy(authTag.begin(), authTag.begin() + AUTH_TAG_SIZE, m_serverDataFrame->s_authTag);
    }

    m_serverDataFrame->s_endMarker = SERVER_FRAME_END_MAKER;
    auto endTime = std::chrono::steady_clock::now();
    double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    PLAT_LOG_D("-- Server data frame constructed in %.4f ms", elapsedTime);
}

void Transmission::ServerFrame::ServerFrame::currentSequenceNumber()
{
    if(m_serverDataFrame->s_sequenceNumber >= ServerFrameConstants::SERVER_FRAME_SEQUENCE_NUMBER) 
    {
        m_serverDataFrame->s_sequenceNumber = ServerFrameConstants::RESET_SEQUENCE;
    }
}

void Transmission::ServerFrame::ServerFrame::sendDataFrameToServer(std::shared_ptr<MQTT> mqtt,
                                                                 const std::vector<unsigned char>& nonce,
                                                                 unsigned long long ciphertextLength,
                                                                 const std::vector<unsigned char>& ciphertext,
                                                                 const std::vector<unsigned char>& authTag)
{
    // PLAT_LOG_D("-- Frame buffer size: %d", ciphertextLength);
    constructServerDataFrame(nonce, ciphertextLength, ciphertext, authTag);
    // for(int i = 0; i < 16; i++) {
    //     PLAT_LOG_ED("%02X", ciphertext[i]);
    // }
    // PLAT_LOG_ED("\n");
    // for(int i = 0; i < 16; i++) {
    //     PLAT_LOG_ED("%02X", nonce[i]);
    // }
    // PLAT_LOG_ED("\n");
    auto startTime = std::chrono::steady_clock::now();
    mqtt->publishData(m_serverDataFrame.get(), sizeof(DataFrame::ServerFrameData));
    // PLAT_LOG_D("-- Publish data size: %d", sizeof(DataFrame::ServerFrameData));
    auto endTime = std::chrono::steady_clock::now();
    double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    PLAT_LOG_D("-- Sent server data frame to server in %.4f ms", elapsedTime);
}

int16_t Transmission::ServerFrame::ServerFrame::getSequenceNumber()
{
    return m_serverDataFrame->s_sequenceNumber;
}

int Transmission::ServerFrame::ServerFrame::isPacketFromServerReached(std::shared_ptr<MQTT> mqtt)
{
    auto startTime = std::chrono::steady_clock::now();
    constexpr auto timeout = std::chrono::seconds(Timer::TIMEOUT_FOR_COMMUNICATION);
    while(!mqtt->m_mqttIsAckPackageArrived && !mqtt->m_mqttIsSequenceNumberNeededUpdate)
    {
        mqtt->connect();
        if (std::chrono::steady_clock::now() - startTime >= timeout)
        {
            mqtt->m_mqttIsTimeout = true;
            return SERVER_MQTT_FAILED; 
        }
    }
    if(mqtt->m_mqttIsAckPackageArrived){
        mqtt->m_mqttIsTimeout = false;
        mqtt->m_mqttIsAckPackageArrived = false;
        return SERVER_RECEIVE_ACK;
    }
    else if(mqtt->m_mqttIsSequenceNumberNeededUpdate){
        PLAT_LOG_D(__FMT_STR__, "-- Attack detected! Received signal from server to update sequence number");
        mqtt->m_mqttIsTimeout = false;
        mqtt->m_mqttIsSequenceNumberNeededUpdate = false;
        return SERVER_RECEIVE_SEQUENCE_NUMBER;
    }
    return SERVER_MQTT_FAILED;
}

std::vector<unsigned char>& Transmission::ServerFrame::ServerFrame::getSecretKeyComputed()
{
    return m_secretKeyComputed;
}

void Transmission::ServerFrame::ServerFrame::resetSequenceNumber()
{
    m_serverDataFrame->s_sequenceNumber = ServerFrameConstants::INITIAL_SEQUENCE;
}

int Transmission::ServerFrame::ServerFrame::getSafeCounter()
{
    return m_safeCounter;
}

void Transmission::ServerFrame::ServerFrame::setSafeCounter(int safeCounter)
{
    m_safeCounter = safeCounter;
}

void Transmission::ServerFrame::ServerFrame::setSequenceNumber(int sequenceNumber)
{
    m_serverDataFrame->s_sequenceNumber = sequenceNumber;
}
