
#ifndef __PROTOCOL__
#define __PROTOCOL__

#include <stdint.h>
#include <vector>

#define FRAME_SIZE 256
#define FRAME_SIZE_BYTES FRAME_SIZE / 8

#define CTRL_MODE (uint8_t)(0b0)
#define DATA_MODE (uint8_t)(0b1)

#define ACK (uint8_t)(0b1111111)
#define NAK_SEND (uint8_t)(0b0000000) // only for sending

#define RTS (uint8_t)(0b1010101)
#define CTS (uint8_t)(0b0101010)

#define DATA_START (uint8_t)(0b0000000)
#define DATA_DONE (uint8_t)(0b0110011)
#define EOT (uint8_t)(0b1100011)

// TODO: make class instead of struct
typedef struct
{
    uint8_t mode;
    uint8_t header;
    uint16_t data_len;
    uint8_t data[FRAME_SIZE_BYTES];
} frame;

uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n, uint8_t *data_n);
uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n);
uint8_t packFrame(std::vector<uint8_t> &signal, frame &frame);
uint8_t packetFromFrame(std::vector<uint8_t> &packet, frame &frame);
uint8_t printFrame(frame &frame);

#endif // __PROTOCOL__