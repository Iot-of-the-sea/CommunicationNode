#include "control.h"

static random_device rd;
static mt19937 gen(rd());
static uniform_int_distribution<uint8_t> dist(0, 127); // 7-bit numbers

uint8_t isAck(string &packet)
{
    if (packet.size() < 1)
        return false;

    uint8_t headerBuf, err;
    err = getHeaderByte(packet, headerBuf);
    return err == NO_ERROR && isAck(headerBuf);
}

uint8_t isAck(uint8_t header)
{
    return header == 0x7f;
}

uint8_t getHeaderByte(const string &packet, uint8_t &headerBuf)
{
    if (packet.size() < 1)
        return EMPTY_PACKET_ERROR;

    headerBuf = (uint8_t)packet[0];
    return NO_ERROR;
}

// https://chatgpt.com/share/68016b3a-2604-800e-b2e3-61e535d53a15
uint8_t generate_parity_byte(bool even)
{
    uint8_t random7 = dist(gen);                // Generate 7 random bits
    uint8_t count = bitset<7>(random7).count(); // Count number of 1s

    uint8_t parity_bit = (count % 2 == (even ? 0 : 1)) ? 0 : 1; // Determine 8th bit
    return (random7 << 1) | parity_bit;                         // Combine to form 8-bit number
}

uint8_t check_byte_even_parity(uint8_t byte)
{
    uint8_t count = bitset<8>(byte).count(); // Count number of 1s
    return count % 2 == 0;
}