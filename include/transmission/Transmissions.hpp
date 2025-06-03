#include "frameProtocol/uartFrame/UartFrame.hpp"
#include "frameProtocol/serverFrame/ServerFrame.hpp"
#include "setupConfiguration/SetupNumberHelper.hpp"
#include "asconCryptography/Ascon128a.hpp"
#include "communication/MQTT.hpp"

class Transmissions
{
private:
    std::shared_ptr<Transmission::UartFrame::UartFrame> m_uart;
    std::shared_ptr<Cryptography::Ascon128a> m_ascon128a;
    std::shared_ptr<Transmission::ServerFrame::ServerFrame> m_server;
    std::shared_ptr<MQTT> m_mqtt;
    TransmissionState m_transmissionFinalState;
    std::vector<uint8_t> m_data;
    std::vector<uint8_t> m_nonce;
    std::vector<uint8_t> m_authTag;
    unsigned long long m_dataLength;
    bool m_isHandshake = false;
    int m_countPacket = 0;
    int m_safeCounter = 0;
    bool m_isTransmissionSucceed = true;
public:
    TransmissionState m_transmissionNextState;
    double m_frameProcessTime;
    double m_handshakeProcessTime;
    double m_encryptionProcessTime;
    double m_sendDataProcessTime;
    double m_ackResponseTime;
    bool m_isFrameParsing = false;
    /**
     * @brief Constructor of Transmission
     */
    Transmissions();

    /**
     * @brief Destructor of Transmission
     */
    ~Transmissions();

    /**
     * @brief Smart pointer to create Transmission object
     */
    static std::shared_ptr<Transmissions> create();

    std::shared_ptr<Transmission::UartFrame::UartFrame> getUart();

    /**
     * @brief Reset the state machine
     */
    void resetStateMachine();

    /**
     * @brief Handle transmission error
     */
    void handleTransmissionError();

    /**
     * @brief Reset parser state
     */
    void resetTransmissionState();

    /**
     * @brief Start transmission 
     * @brief First, start to parse the frame to get data
     * @brief Process handshake using EDHC to get key for encryption
     * @brief Encrypt the data using Ascon128a
     * @brief Send the data to server
     */
    bool startTransmissionProcess();

    /**
     * @brief update sequence number when receive signal from server
     */
    void updateSequenceNumber(std::shared_ptr<Transmission::ServerFrame::ServerFrame> &server);

    void setSafeCounter(int safeCounter);

    /**
     * @brief Loop the MQTT connection
     */
    void loopMqtt();

    std::shared_ptr<Transmission::ServerFrame::ServerFrame> getServer() {
        return m_server;
    }
    
    std::shared_ptr<MQTT> getMqtt() {
        return m_mqtt;
    }
};