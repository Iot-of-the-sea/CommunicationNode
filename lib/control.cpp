#include "control.h"

uint8_t isAck(string &header)
{
    if (header.size() != 1)
        return false;

    return (uint8_t)header[0] == 0x7f;
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