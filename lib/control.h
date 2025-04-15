#ifndef __CONTROL__
#define __CONTROL__

#include <string>

#define NO_ERROR 0x00
#define EMPTY_PACKET_ERROR 0x01

using namespace std;

uint8_t isAck(string &header);

uint8_t isAck(uint8_t header);

uint8_t getHeaderByte(const string &packet, uint8_t &headerBuf);

#endif // __CONTROL__