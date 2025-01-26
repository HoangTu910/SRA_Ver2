#pragma once

#include <stdint.h>
#include <Arduino.h> 

class Platform{
public:
    static uint32_t GetCurrentTimeMs();
};
