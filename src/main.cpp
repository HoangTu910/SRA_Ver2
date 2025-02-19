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
}

void loop() {
    controller->loopMqtt();
    if(controller->m_transmissionNextState == TransmissionState::PROCESS_FRAME_PARSING)
    {
        startTime = std::chrono::high_resolution_clock::now();
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
        PLAT_LOG_D("S: %d - L: %d", packetSuccess, packetLoss);
        PLAT_LOG_D("AVG: %.2f ms", totalTime / packetSuccess);
    }
    
    // __AIOT_FOR_MEDTECH_DESLAB__;
}


// Test::UartFrameTest::frameParserTest(); // Frame test passed
// Test::Ascon128aTest::RunAscon128aTest(); // Ascon test passed