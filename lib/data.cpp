#include <iostream>
#include <fstream>
#include <string>
#include "protocol.h"

uint8_t packFrame(std::vector<uint8_t> &signal, frame &frame)
{
    uint8_t headerByte = frame.header | (frame.mode << 7);

    if (frame.mode == CTRL_MODE)
        signal.resize(1);
    else
    {
        signal.resize(33);
        for (size_t byte = 0; byte < 32; byte++)
        {
            signal.at(byte + 1) = frame.data[byte];
        }
    }

    signal.at(0) = headerByte;

    return 0;
}

// includes CRC8
uint8_t packetFromFrame(std::vector<uint8_t> &packet, frame &frame)
{
    packFrame(packet, frame);
    packet.push_back(CRC8); // TODO: test if this is doing anything
}

uint8_t test_main()
{
    std::ifstream file("01102521.csv"); // Open the file
    if (!file)
    {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line))
    { // Read line by line
        std::cout << line << std::endl;
    }

    file.close(); // Close the file
    return 0;
}
