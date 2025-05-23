
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
#define HEADER_DATA (uint8_t)(0b0011100)
#define EOT (uint8_t)(0b1100011)

// for when transmit or receive is done
#define HAS_FILES (uint8_t)(0b0001111)
#define HAS_NO_FILES (uint8_t)(0x1110000)

// TODO: make class instead of struct
typedef struct
{
    uint8_t mode;
    uint8_t header;
    uint16_t data_len = FRAME_SIZE_BYTES;
    uint8_t data[FRAME_SIZE_BYTES] = {0};
} frame;

typedef struct
{
    uint16_t nodeId = 0;
    uint32_t fileSizeBytes = 0;
} headerData;

uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n, uint8_t *data_n);
uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n);
uint8_t packFrame(std::vector<uint8_t> &signal, frame &frame);
uint8_t packetFromFrame(std::vector<uint8_t> &packet, frame &frame);
uint8_t packetFromHeaderData(std::vector<uint8_t> &packet, headerData &header);
uint8_t frameFromHeaderData(frame &frame, headerData &header);
uint8_t printFrame(frame &frame);

#endif // __PROTOCOL__