#include <stdint.h>
#include <cstddef>

/*
@brief CRC16 class to calculate CRC16 checksum
*/

class CRC16
{
private:
    static const uint16_t POLYNOMIAL = 0x11021;  // CRC-16-CCITT polynomial
    static const uint16_t INITIAL_VALUE = 0xFFFF;  // Initial value for CRC-16
public:
    static uint16_t calculateCRC(const uint8_t* data, size_t length);
};


