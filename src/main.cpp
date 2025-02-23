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

auto wifi = Wifi::create(WifiHelper::SSID, WifiHelper::PASSWORD);
auto ascon128a = Cryptography::Ascon128a::create();
auto controller = Transmissions::create();

int packetSuccess = 0;
int packetLoss = 0;

static double totalTime = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

void setup() {
    setCpuFrequencyMhz(240);
    Serial.begin(Serial::BAUD_RATE);
    wifi->connect();
    controller->m_frameProcessTime = 0;
    controller->m_handshakeProcessTime = 0;
    controller->m_encryptionProcessTime = 0;
    controller->m_sendDataProcessTime = 0;
    controller->m_ackResponseTime = 0;
}

void loop() {
    static std::vector<int> packetsPerMinute;
    static std::vector<double> pdrPerMinute;
    static std::vector<double> frameTimePerMinute;
    static std::vector<double> handshakeTimePerMinute;
    static std::vector<double> encryptTimePerMinute;
    static std::vector<double> sendDataPerMinute;
    static std::vector<double> ackResponsePerMinute;
    static int currentMinute = 0;
    static int packetsThisMinute = 0;
    static int totalPacketsThisMinute = 0;
    static unsigned long minuteTimer = millis();

    // Regular transmission code
    controller->loopMqtt();
    if(controller->m_transmissionNextState == TransmissionState::PROCESS_FRAME_PARSING) {
        startTime = std::chrono::high_resolution_clock::now();
        totalPacketsThisMinute++;
    }
    controller->startTransmissionProcess();
    
    if (controller->m_transmissionNextState == TransmissionState::TRANSMISSION_ERROR && controller->m_isFrameParsing == true) {
        packetLoss++;
        controller->m_isFrameParsing = false;
        PLAT_LOG_D("S: %d - L: %d", packetSuccess, packetLoss);
    }
    else if (controller->m_transmissionNextState == TransmissionState::TRANSMISSION_COMPLETE) {
        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        totalTime += elapsedTime;
        packetSuccess++;
        packetsThisMinute++;
        PLAT_LOG_D("%d  %d", packetSuccess, packetLoss);
        PLAT_LOG_D("%.2f ms", totalTime / packetSuccess);
    }

    // Check if a minute has passed
    if (millis() - minuteTimer >= 60000) {
        packetsPerMinute.push_back(packetsThisMinute);
        double pdr = totalPacketsThisMinute > 0 ? (double)packetsThisMinute / totalPacketsThisMinute : 0;
        pdrPerMinute.push_back(pdr);
        frameTimePerMinute.push_back(totalPacketsThisMinute > 0 ? (double)controller->m_frameProcessTime / totalPacketsThisMinute : 0);
        handshakeTimePerMinute.push_back(totalPacketsThisMinute > 0 ? (double)controller->m_handshakeProcessTime / totalPacketsThisMinute : 0);
        encryptTimePerMinute.push_back(totalPacketsThisMinute > 0 ? (double)controller->m_encryptionProcessTime / totalPacketsThisMinute : 0);
        sendDataPerMinute.push_back(totalPacketsThisMinute > 0 ? (double)controller->m_sendDataProcessTime / totalPacketsThisMinute : 0);
        ackResponsePerMinute.push_back(totalPacketsThisMinute > 0 ? (double)controller->m_ackResponseTime / totalPacketsThisMinute : 0);
        currentMinute++;
        packetsThisMinute = 0;
        totalPacketsThisMinute = 0;
        minuteTimer = millis();
        // Reset process times
        controller->m_frameProcessTime = 0;
        controller->m_handshakeProcessTime = 0;
        controller->m_encryptionProcessTime = 0;
        controller->m_sendDataProcessTime = 0;
        controller->m_ackResponseTime = 0;

        // After 2 hours (120 minutes), print results
        if (currentMinute >= 120) {
            Serial.println("Packets per minute:");
            for (size_t i = 0; i < packetsPerMinute.size(); i++) {
                Serial.printf("Minute %d: %d packets\n", i + 1, packetsPerMinute[i]);
            }
            
            Serial.println("\nPDR per minute:");
            for (size_t i = 0; i < pdrPerMinute.size(); i++) {
                Serial.printf("Minute %d: %.2f%%\n", i + 1, pdrPerMinute[i] * 100);
            }

            Serial.println("\nProcess times per minute:");
            for (size_t i = 0; i < frameTimePerMinute.size(); i++) {
                Serial.printf("Minute %d:\n", i + 1);
                Serial.printf("  Frame Time: %.2f ms\n", frameTimePerMinute[i]);
                Serial.printf("  Handshake Time: %.2f ms\n", handshakeTimePerMinute[i]);
                Serial.printf("  Encryption Time: %.2f ms\n", encryptTimePerMinute[i]);
                Serial.printf("  Send Data Time: %.2f ms\n", sendDataPerMinute[i]);
                Serial.printf("  ACK Response Time: %.2f ms\n", ackResponsePerMinute[i]);
            }
            
            while(1); // Stop after 2 hours
        }
    }
    // delay(100);
}


// Test::UartFrameTest::frameParserTest(); // Frame test passed
// Test::Ascon128aTest::RunAscon128aTest(); // Ascon test passed