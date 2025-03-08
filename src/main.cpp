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
auto uart = Transmission::UartFrame::UartFrame::create();
// HardwareSerial mySerial(2);

uint8_t testPacket[51] = {
    0x01, // Packet type
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Secret key (8 bytes shown, extend to 48)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, // CRC (2 bytes)
};

void setup() {
    setCpuFrequencyMhz(240);
    // mySerial.begin(115200, SERIAL_8N1, Serial::TX_PIN, Serial::RX_PIN);
    Serial.begin(Serial::BAUD_RATE);
    wifi->connect();
}

void loop() {
    controller->loopMqtt();
    controller->startTransmissionProcess();
    // uart->transmitData(testPacket);
    __AIOT_FOR_MEDTECH_DESLAB__;
}


// Test::UartFrameTest::frameParserTest(); // Frame test passed
// Test::Ascon128aTest::RunAscon128aTest(); // Ascon test passed