#include "frameProtocol/serverFrame/ServerFrame.hpp"

Transmission::ServerFrame::ServerFrame::ServerFrame()
{
    m_handshakeFrame = std::make_shared<Handshake::HandshakeFrameData>();
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
            PLAT_PRINT_BYTES("Public key generated", ECDH::devicePublicKey, ECC_PUB_KEY_SIZE);
            PLAT_PRINT_BYTES("Private key generated", ECDH::devicePrivateKey, ECC_PUB_KEY_SIZE);
            PLAT_LOG_D(__FMT_STR__, "[HANDSHAKE STATE] - GENERATE PUBLIC KEY");
            m_handshakeNextState = HandshakeState::CONSTRUCT_PUBLIC_KEY_FRAME;
            break;
        }
        case HandshakeState::CONSTRUCT_PUBLIC_KEY_FRAME:
        {
            m_handshakeFrame->s_preamble = SERVER_FRAME_PREAMBLE;
            m_handshakeFrame->s_identifierId = SERVER_FRAME_IDENTIFIER_ID;
            m_handshakeFrame->s_packetType = SERVER_FRAME_PACKET_HANDSHAKE_TYPE;
            m_handshakeFrame->s_sequenceNumber = SERVER_FRAME_SEQUENCE_NUMBER;
            memcpy(m_handshakeFrame->s_authTag, SERVER_FRAME_AUTH_TAG, sizeof(m_handshakeFrame->s_authTag));
            memcpy(m_handshakeFrame->s_publicKey, ECDH::devicePublicKey, ECC_PUB_KEY_SIZE);
            PLAT_LOG_D(__FMT_STR__, "[HANDSHAKE STATE] - CONSTRUCT PUBLIC KEY FRAME");
            m_handshakeNextState = HandshakeState::SEND_PUBLIC_KEY_FRAME;
            break;
        }
        case HandshakeState::SEND_PUBLIC_KEY_FRAME:
        {
            mqtt->publishData(m_handshakeFrame.get(), sizeof(Handshake::HandshakeFrameData));
            PLAT_LOG_D(__FMT_STR__, "[HANDSHAKE STATE] - SEND PUBLIC KEY FRAME");
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
                m_handshakeNextState = HandshakeState::HANDSHAKE_COMPLETE;
                PLAT_LOG_D(__FMT_STR__, "Public key received, proceeding to HANDSHAKE_COMPLETE");
            }
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
