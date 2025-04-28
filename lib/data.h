
#ifndef __DATA__
#define __DATA__

#include <iostream>
#include <fstream>
#include <cstring>
#include "crc8.h"
#include "protocol.h"
#include "control.h"

using namespace std;

class FileWriter
{
private:
    string _fileName;
    ofstream _writeStream;

    uint8_t write(char *data, uint32_t length);

public:
    FileWriter() {};

    FileWriter(const char *fileName);

    FileWriter(string fileName);

    uint8_t open();

    uint8_t close();

    uint8_t write(char *data);
    uint8_t write(string data);
    uint8_t write(uint8_t *data, uint32_t length);

    string getFileName() { return _fileName; }
};

crc_t find_crc(string &packet);
crc_t find_crc(vector<uint8_t> &packet);
uint8_t check_received_crc(string packet);
uint8_t get_packet_data(string &packet, string &data);

#endif // __DATA__