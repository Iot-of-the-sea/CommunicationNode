#include "data.h"

uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n, uint8_t *data_n)
{
    updateFrame(frame, mode_n, header_n);
    std::memcpy(frame.data, data_n, FRAME_SIZE_BYTES);
    return 0;
}

uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n)
{
    frame.mode = mode_n;
    frame.header = header_n;
    return 0;
}

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

    return 0;
}

uint8_t printFrame(frame &frame)
{
    char chunk[10];
    std::string out;
    snprintf(chunk, 10, "%d | %x |", frame.mode == DATA_MODE, frame.header);
    out = chunk;

    for (int i = 0; i < FRAME_SIZE_BYTES; i++)
    {
        snprintf(chunk, 10, " %x ", frame.data[i]);
        out += chunk;
    }

    std::cout << out << std::endl;

    return 0;
}

// int test()
// {
//     std::ifstream ifile("./lib/01102521.csv", std::ifstream::binary); // Open the file
//     std::ofstream ofile("./lib/out.txt", std::ifstream::binary);

//     if (!(ifile && ofile))
//     {
//         std::cerr << "Error opening file!" << std::endl;
//         return 1;
//     }

//     char *frameBuf = new char[FRAME_SIZE_BYTES];
//     frame dataFrame = {
//         .mode = DATA_MODE,
//         .header = 0,
//         .data = {}};
//     while (ifile.read(frameBuf, FRAME_SIZE_BYTES))
//     {
//         std::memcpy(dataFrame.data, reinterpret_cast<uint8_t *>(frameBuf), FRAME_SIZE_BYTES);
//         printFrame(dataFrame);
//         ofile.write(frameBuf, FRAME_SIZE_BYTES);
//         dataFrame.header++;
//     }
//     ofile.write(frameBuf, ifile.gcount());

//     ifile.close(); // Close the file
//     ofile.close(); // Close the file
//     return 0;
// }
