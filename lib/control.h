#ifndef __CONTROL__
#define __CONTROL__

#include <string>
#include <random>
#include <bitset>

// TODO: move errors to another file?
#define NO_ERROR 0x00
#define EMPTY_PACKET_ERROR 0x01
#define STATE_ERROR 0x02
#define TIMEOUT_ERROR 0x03
#define IO_ERROR 0x04
#define ARGUMENT_ERROR 0x05
#define MEMORY_ERROR 0x06
#define GPIO_ERROR 0x07

using namespace std;

uint8_t isAck(string &packet);

uint8_t isAck(uint8_t header);

uint8_t getHeaderByte(const string &packet, uint8_t &headerBuf);

uint8_t generate_parity_byte(bool even);

uint8_t check_byte_even_parity(uint8_t byte);

#endif // __CONTROL__