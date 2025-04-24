#ifndef __CONTROL__
#define __CONTROL__

#include <string>
#include <random>
#include <bitset>

#define NO_ERROR 0x00
#define EMPTY_PACKET_ERROR 0x01

using namespace std;

uint8_t isAck(string &packet);

uint8_t isAck(uint8_t header);

uint8_t getHeaderByte(const string &packet, uint8_t &headerBuf);

uint8_t generate_parity_byte(bool even);

uint8_t check_byte_even_parity(uint8_t byte);

#endif // __CONTROL__