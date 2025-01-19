#include "CRC16.hpp"

uint16_t CRC16::calculateCRC(const uint8_t *data, size_t length)
{
    uint16_t crc = CRC16::INITIAL_VALUE;

    for (size_t i = 0; i < length; ++i)
    {
        crc ^= (data[i] << 8);  
        for (int j = 8; j > 0; --j)  
        {
            if (crc & 0x8000)  
            {
                crc = (crc << 1) ^ CRC16::POLYNOMIAL;
            }
            else
            {
                crc = (crc << 1);  
            }
        }
    }

    return crc;
}
