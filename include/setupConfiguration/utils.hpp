#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h> // For Serial.println, etc.

static void PLAT_LOG_D(const char* message);
static void PLAT_LOG_D(const char* format, ...);

#endif  // UTILS_H
