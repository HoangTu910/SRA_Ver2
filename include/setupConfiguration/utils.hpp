#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h> // For Serial.println, etc.
#include <cstdarg>

#define ANSI_COLOR_GREEN "\033[32m"
#define ANSI_COLOR_RED   "\033[31m"
#define ANSI_COLOR_RESET "\033[0m"
#define LOG_BUFFER 512
#define __FMT_STR__ "%s"
#define __FMT_INT__ "%d"
#define __FMT_FLOAT__ "%f"

// PLAT_ASSERT macro
#define PLAT_ASSERT(condition, format, ...) \
    do { \
        if (!(condition)) { \
            PLAT_LOG_ED("ASSERT FAILED: "); \
            PLAT_LOG_D(format, ##__VA_ARGS__); \
            PLAT_LOG_D("File: %s, Line: %d", __FILE__, __LINE__); \
            exit(EXIT_FAILURE);   \
        } \
    } while (0)

#define PLAT_ASSERT_NULL(condition, format, ...) \
    do { \
        if ((condition) == nullptr) { \
            PLAT_LOG_ED("ASSERT FAILED: "); \
            PLAT_LOG_D(format, ##__VA_ARGS__); \
            PLAT_LOG_D("File: %s, Line: %d", __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

void PLAT_LOG_D(const char* message);
void PLAT_LOG_D(const char* format, ...);
void PLAT_LOG_ED(const char* format, ...);
void PLAT_WRITE_LOG(const char* format, ...);
void TEST_PASSED(const char* message);
void TEST_FAILED(const char* message);
void TEST_RESULT(bool passed);

#endif  // UTILS_H
