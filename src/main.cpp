/********** TEST ZONE *************/
#include "test/frameProtocol/uartFrame/uartFrameTest.hpp"
#include "test/asconCryptography/Ascon128aTest.hpp"
/********** TEST ZONE *************/

#include <Arduino.h>
#include <esp32-hal-cpu.h>
#include "setupConfiguration/utils.hpp"
#include "setupConfiguration/SetupNumberHelper.hpp"
#include "communication/Wifi.hpp"
#include "setupConfiguration/utils.hpp"
#include "transmission/Transmissions.hpp"

typedef struct IGNORE_PADDING MetricsFrame
{
    uint16_t str_header;
    uint32_t str_identifierId;
    uint8_t str_packetType;
    uint8_t str_metrics[METRICS_SIZE];
    uint16_t str_trailer;
} MetricsFrame;

auto wifi = Wifi::create(WifiHelper::SSID, WifiHelper::PASSWORD);
auto ascon128a = Cryptography::Ascon128a::create();
auto controller = Transmissions::create();
auto uart = Transmission::UartFrame::UartFrame::create();

unsigned long lastMetricsTime = 0;
int packetSuccess = 0;
int packetLoss = 0;
double totalTime = 0;
int packetsThisMinute = 0;
auto startTime = std::chrono::high_resolution_clock::now();

void publishMetrics(std::shared_ptr<MQTT> &m_mqtt) {
    if (!m_mqtt) {
        PLAT_LOG_D(__FMT_STR__, "-- Error: null MQTT pointer");
        return;
    }

    MetricsFrame frame;
    
    frame.str_header = SERVER_FRAME_PREAMBLE;
    frame.str_packetType = SERVER_FRAME_PACKET_METRICS_TYPE;
    frame.str_identifierId = SERVER_FRAME_IDENTIFIER_ID;

    int totalPackets = packetSuccess + packetLoss;
    float pdr = totalPackets > 0 ? (float)packetSuccess / totalPackets * 100 : 0;
    float avgLatency = packetSuccess > 0 ? totalTime / packetSuccess : 0;

    frame.str_metrics[0] = (uint8_t)(pdr); 
    frame.str_metrics[1] = (uint8_t)(avgLatency); 
    
    frame.str_trailer = SERVER_FRAME_END_MAKER;
    
    m_mqtt->publishData((uint8_t*)&frame, sizeof(MetricsFrame));
}

void setup() {
    setCpuFrequencyMhz(MAX_FREQUENCY);
    Serial.begin(Serial::BAUD_RATE);
    wifi->connect();
}

void loop() {
    controller->loopMqtt();

    if (controller->m_transmissionNextState == TransmissionState::HANDSHAKE_AND_KEY_EXCHANGE) {
        startTime = std::chrono::high_resolution_clock::now();
    }
    
    controller->startTransmissionProcess();
    
    if (controller->m_transmissionNextState == TransmissionState::TRANSMISSION_ERROR && controller->m_isFrameParsing == true) {
        packetLoss++;
        controller->m_isFrameParsing = false;
    }
    else if (controller->m_transmissionNextState == TransmissionState::TRANSMISSION_COMPLETE) {
        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        totalTime += elapsedTime;
        packetSuccess++;
        packetsThisMinute++;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastMetricsTime >= VERY_FAST_MILI && (controller->m_transmissionNextState == TransmissionState::TRANSMISSION_COMPLETE)) { 
        PLAT_LOG_D("[SPECIAL PACKET PER MINUTE] SENDING METRICS: PDR: %.2f%%, AVG LATENCY: %.2f ms]", 
            (float)packetSuccess / (packetSuccess + packetLoss) * 100, 
            packetSuccess > 0 ? totalTime / packetSuccess : 0);
        auto mqtt = controller->getMqtt();
        publishMetrics(mqtt);
        lastMetricsTime = currentTime;
        packetsThisMinute = 0;
        __AIOT_FOR_MEDTECH_DESLAB__;
    }
}
