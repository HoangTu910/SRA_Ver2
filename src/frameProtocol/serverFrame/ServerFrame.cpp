#include "frameProtocol/serverFrame/ServerFrame.hpp"

Transmission::ServerFrame::ServerFrame::ServerFrame() : m_secretKeyComputed(ECC_PUB_KEY_SIZE) 
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
    switch(m_handshakeNextState)
    {
        case HandshakeState::GENERATE_PUBLIC_KEY:
        {
            static int initialized = 0;
            if (!initialized)
            {
                prng_init((0xbad ^ 0xc0ffee ^ 42) | 0xcafebabe | 666);
                initialized = 1;
            }
            for (int i = 0; i < ECC_PRV_KEY_SIZE; ++i)
            {
                ECDH::devicePrivateKey[i] = prng_next();
            }
            assert(ECDH::ecdh_generate_keys(ECDH::devicePublicKey, ECDH::devicePrivateKey) == 1);
            // PLAT_PRINT_BYTES("Public key generated", ECDH::devicePublicKey, ECC_PUB_KEY_SIZE);
            // PLAT_PRINT_BYTES("Private key generated", ECDH::devicePrivateKey, ECC_PRV_KEY_SIZE);
            // PLAT_LOG_D(__FMT_STR__, "-- Generated public key");
            m_handshakeNextState = HandshakeState::CONSTRUCT_PUBLIC_KEY_FRAME;
            break;
        }
        case HandshakeState::CONSTRUCT_PUBLIC_KEY_FRAME:
        {
            m_handshakeFrame->s_preamble = SERVER_FRAME_PREAMBLE;
            m_handshakeFrame->s_identifierId = SERVER_FRAME_IDENTIFIER_ID;
            m_handshakeFrame->s_packetType = SERVER_FRAME_PACKET_HANDSHAKE_TYPE;
            m_handshakeFrame->s_sequenceNumber = SERVER_FRAME_SEQUENCE_NUMBER;
            m_handshakeFrame->s_endMarker = SERVER_FRAME_END_MAKER;
            memcpy(m_handshakeFrame->s_publicKey, ECDH::devicePublicKey, ECC_PUB_KEY_SIZE);
            // PLAT_LOG_D(__FMT_STR__, "-- Contructed public key frame");
            m_handshakeNextState = HandshakeState::SEND_PUBLIC_KEY_FRAME;
            break;
        }
        case HandshakeState::SEND_PUBLIC_KEY_FRAME:
        {
            mqtt->publishData(m_handshakeFrame.get(), sizeof(Handshake::HandshakeFrameData));
            // PLAT_LOG_D(__FMT_STR__, "-- Sent public key frame to server");
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
            }

            if (mqtt->m_mqttIsMessageArrived) {
                mqtt->m_mqttIsMessageArrived = false; // Reset flag
                m_handshakeNextState = HandshakeState::COMPUTE_SHARED_SECRET;
                // PLAT_LOG_D(__FMT_STR__, "-- Received public key from server");
            }
            break;
        }
        case HandshakeState::COMPUTE_SHARED_SECRET:
        {
            assert(ECDH::ecdh_shared_secret(ECDH::devicePrivateKey, mqtt->m_mqttCallBackDataReceive, m_secretKeyComputed.data()));
            // printbytes("Secret key generated", m_secretKeyComputed, ECC_PUB_KEY_SIZE);
            // PLAT_LOG_D(__FMT_STR__, "-- Handshake completed");
            m_handshakeNextState = HandshakeState::HANDSHAKE_COMPLETE;
            break;
        }
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
                                                                    const std::vector<unsigned char>& cipherText)
{
    // Set frame header fields
    // PLAT_LOG_D(__FMT_STR__, "-- Constructing server data frame");
    m_serverDataFrame->s_preamble = SERVER_FRAME_PREAMBLE;
    m_serverDataFrame->s_identifierId = SERVER_FRAME_IDENTIFIER_ID;  
    m_serverDataFrame->s_packetType = SERVER_FRAME_PACKET_DATA_TYPE;
    currentSequenceNumber();
    m_serverDataFrame->s_timestamp = std::time(nullptr); // Current timestamp

    // Copy nonce
    if(nonce.size() >= NONCE_SIZE) {
        std::copy(nonce.begin(), nonce.begin() + NONCE_SIZE, m_serverDataFrame->s_nonce);
    }

    // Set payload length and copy encrypted data
    m_serverDataFrame->s_payloadLength = cipherTextLength;
    if(cipherTextLength <= sizeof(m_serverDataFrame->s_encryptedPayload)) {
        std::copy(cipherText.begin(), 
                 cipherText.begin() + cipherTextLength,
                 m_serverDataFrame->s_encryptedPayload);
    }

    // Copy authentication tag
    m_serverDataFrame->s_macTag = Utils::MACCompute();

    m_serverDataFrame->s_endMarker = SERVER_FRAME_END_MAKER;
}

int Transmission::ServerFrame::ServerFrame::currentSequenceNumber()
{
    if(m_serverDataFrame->s_sequenceNumber >= ServerFrameConstants::SERVER_FRAME_SEQUENCE_NUMBER) 
    {
        m_serverDataFrame->s_sequenceNumber = ServerFrameConstants::RESET_SEQUENCE;
        return m_serverDataFrame->s_sequenceNumber;
    }
    return m_serverDataFrame->s_sequenceNumber++;
}

void Transmission::ServerFrame::ServerFrame::sendDataFrameToServer(std::shared_ptr<MQTT> mqtt,
                                                                 const std::vector<unsigned char>& nonce,
                                                                 unsigned long long ciphertextLength,
                                                                 const std::vector<unsigned char>& ciphertext)
{
    // PLAT_LOG_D("-- Frame buffer size: %d", ciphertextLength);
    constructServerDataFrame(nonce, ciphertextLength, ciphertext);
    mqtt->publishData(m_serverDataFrame.get(), sizeof(DataFrame::ServerFrameData));
}

uint16_t Transmission::ServerFrame::ServerFrame::getSequenceNumber()
{
    return m_serverDataFrame->s_sequenceNumber;
}

bool Transmission::ServerFrame::ServerFrame::isAckFromServerArrived(std::shared_ptr<MQTT> mqtt)
{
    auto startTime = std::chrono::steady_clock::now();
    constexpr auto timeout = std::chrono::seconds(Timer::TIMEOUT_FOR_COMMUNICATION);
    while(!mqtt->m_mqttIsAckPackageArrived)
    {
        mqtt->connect();
        if (std::chrono::steady_clock::now() - startTime >= timeout)
        {
            mqtt->m_mqttIsTimeout = true;
            return false; 
        }
    }
    if(mqtt->m_mqttIsAckPackageArrived){
        mqtt->m_mqttIsTimeout = false;
        mqtt->m_mqttIsAckPackageArrived = false;
        return true;
    }
    return false;
}

std::vector<unsigned char>& Transmission::ServerFrame::ServerFrame::getSecretKeyComputed()
{
    return m_secretKeyComputed;
}
