#include "setupConfiguration/utils.hpp"
#include <stdarg.h>  // For va_list, va_start, etc.

/*
@brief Print debug message to serial monitor. Simple to use instead of Serial.print.
*/

static void PLAT_LOG_D(const char* message) {
    Serial.println(message);
}

static void PLAT_LOG_D(const char* format, ...) {
    char buffer[256];  
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.println(buffer);
}
