#include "platform/platform.hpp"

uint32_t Platform::GetCurrentTimeMs()
{
    return millis();
}
