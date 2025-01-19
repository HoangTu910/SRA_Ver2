#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h> // For Serial.println, etc.
#include <cstdarg>

void PLAT_LOG_D(const char* message);
void PLAT_LOG_D(const char* format, ...);
void PLAT_LOG_ED(const char* format, ...);

#endif  // UTILS_H
