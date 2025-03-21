#include "setupConfiguration/utils.hpp"
#include <stdarg.h>  // For va_list, va_start, etc.
#include <iostream>  // For std::cerr
#include "utils.hpp"
/** 
*@brief Print debug message to serial monitor. Simple to use instead of Serial.print.
*/

void PLAT_LOG_D(const char* message) {
    Serial.println(message);
}

void PLAT_LOG_D(const char* format, ...) {
    char buffer[LOG_BUFFER];  
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.println(buffer);
}

void PLAT_LOG_ED(const char* format, ...) {
    char buffer[LOG_BUFFER];  
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
}

void TEST_PASSED(const char* message) {
    PLAT_LOG_D("[== TEST PASSED ==]: %s", message);
}

void TEST_FAILED(const char* message) {
    PLAT_LOG_D("[== TEST FAILED ==]: %s", message);
}

void TEST_RESULT(bool passed) {
    if (passed) {
        TEST_PASSED("Work like a charm!");
    } else {
        TEST_FAILED("Oh fuk!");
    }
}

void PLAT_WRITE_LOG(const char* format, ...) {
    char buffer[LOG_BUFFER];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
}

void PLAT_PRINT_BYTES(const char* text, const uint8_t* b, uint64_t len) {
    char buffer[256]; // Adjust size as needed
    size_t offset = snprintf(buffer, sizeof(buffer), "%s[%" PRIu64 "]\t= {", text, len);
    for (uint64_t i = 0; i < len && offset < sizeof(buffer) - 3; ++i) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "0x%02X%s", b[i], i < len - 1 ? ", " : "");
    }
    snprintf(buffer + offset, sizeof(buffer) - offset, "}");
    PLAT_LOG_D("%s", buffer);
}

// void Utils::assignPublicKeyFromString(const std::string &m_mqttCallBackDataReceive)
// {
//     size_t copySize = std::min(m_mqttCallBackDataReceive.size(), static_cast<size_t>(ECC_PUB_KEY_SIZE));
//     std::memcpy(serverPublicKey, m_mqttCallBackDataReceive.data(), copySize);

//     // Zero out the rest if needed
//     if (copySize < ECC_PUB_KEY_SIZE) {
//         std::memset(serverPublicKey + copySize, 0, ECC_PUB_KEY_SIZE - copySize);
//     }
// }
uint32_t Utils::MACCompute()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    gmtime_r(&now, &timeinfo);

    //Currently use fake data for generating MAC tag    
    // uint8_t minute = timeinfo.tm_min;
    // uint8_t hour = timeinfo.tm_hour;
    // uint8_t day = timeinfo.tm_mday;
    // uint8_t month = timeinfo.tm_mon + 1;    
    // uint16_t year = timeinfo.tm_year + 1900; 

    uint8_t minute = 30;
    uint8_t hour = 12;
    uint8_t day = 15;
    uint8_t month = 6;    
    uint16_t year = 2023;

    // PLAT_LOG_D("Current time: %02d:%02d:%02d %02d/%02d/%04d", hour, minute, timeinfo.tm_sec, day, month, year);
    
    uint32_t T = 0;
    T |= (uint32_t)(minute & 0x3F);
    T |= (uint32_t)(hour & 0x1F) << 6;
    T |= (uint32_t)(day & 0x1F) << 11;
    T |= (uint32_t)(month & 0x0F) << 16;
    T |= (uint32_t)(year & 0xFFF) << 20;

    uint32_t K = 0x24C8E560;
    uint32_t T_low_rotl = (T << 7) | (T >> (32 - 7));
    uint32_t A = T ^ T_low_rotl;
    uint32_t T_high_rotr = (T >> 11) | (T << (32 - 11));
    uint32_t B = T ^ T_high_rotr;
    uint32_t MAC_final = (A ^ B) ^ K;

    // PLAT_LOG_D("T = %u", T);
    // PLAT_LOG_D("T_low_rotl = %u", T_low_rotl);
    // PLAT_LOG_D("A = %u", A);
    // PLAT_LOG_D("T_high_rotr = %u", T_high_rotr);
    // PLAT_LOG_D("B = %u", B);
    // PLAT_LOG_D("MAC_final = %u", MAC_final);

    return MAC_final;
}
