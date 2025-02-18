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
    unsigned char *m_data;
    unsigned long long m_dataLength;
public:
    TransmissionState m_transmissionNextState;
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
     * @brief Loop the MQTT connection
     */
    void loopMqtt();
};