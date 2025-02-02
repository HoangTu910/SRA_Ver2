/********** TEST ZONE *************/
#include "test/frameProtocol/uartFrame/uartFrameTest.hpp"
#include "test/asconCryptography/Ascon128aTest.hpp"
/********** TEST ZONE *************/

#include <Arduino.h>
#include "setupConfiguration/utils.hpp"
#include "setupConfiguration/SetupNumberHelper.hpp"
#include "communication/Wifi.hpp"
#include "setupConfiguration/utils.hpp"
#include "transmission/Transmissions.hpp"

auto wifi = Wifi::create(WifiHelper::SSID, WifiHelper::PASSWORD);
auto ascon128a = Cryptography::Ascon128a::create();
auto controller = Transmissions::create();

void setup() {
    Serial.begin(Serial::BAUD_RATE);
    wifi->connect();
}

void loop() {
    controller->loopMqtt();
    controller->startTransmissionProcess();
    __AIOT_FOR_MEDTECH_DESLAB__;
}

// Test::UartFrameTest::frameParserTest(); // Frame test passed
// Test::Ascon128aTest::RunAscon128aTest(); // Ascon test passed