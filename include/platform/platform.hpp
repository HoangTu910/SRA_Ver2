#pragma once

#include <stdint.h>
#include <Arduino.h> 

enum Timer
{
    TIMEOUT_FOR_COMMUNICATION = 5
};

class Platform{
public:
    static uint32_t GetCurrentTimeMs();
};
