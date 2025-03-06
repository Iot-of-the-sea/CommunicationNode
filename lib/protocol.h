
#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <stdint.h>
#include <vector>

#define CRC8 (uint8_t)(0x07)

#define CTRL_MODE (uint8_t)(0b0)
#define DATA_MODE (uint8_t)(0b1)

#define ACK (uint8_t)(0b1111111)
// #define NAK

#define RTS (uint8_t)(0b1010101)
#define CTS (uint8_t)(0b0101010)

#define DATA_START (uint8_t)(0b0000000)
#define DATA_DONE (uint8_t)(0b0110011)
#define EOT (uint8_t)(0b1100011)

typedef struct
{
    uint8_t mode;
    uint8_t header;
    uint8_t data[32];
} frame;

uint8_t packFrame(std::vector<uint8_t> &signal, frame &frame);

#endif // __PROTOCOL__