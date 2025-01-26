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
