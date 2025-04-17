
#ifndef __DATA__
#define __DATA__

#include <iostream>
#include <fstream>
#include <cstring>
#include "crc8.h"
#include "protocol.h"

using namespace std;

crc_t find_crc(string &packet);
crc_t find_crc(vector<uint8_t> &packet);
uint8_t check_received_crc(string packet);

#endif // __DATA__