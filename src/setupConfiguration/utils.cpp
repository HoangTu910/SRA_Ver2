#include "setupConfiguration/utils.hpp"
#include <stdarg.h>  // For va_list, va_start, etc.
#include <iostream>  // For std::cerr
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