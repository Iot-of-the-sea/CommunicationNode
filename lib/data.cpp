#include "data.h"

uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n, uint8_t *data_n)
{
    updateFrame(frame, mode_n, header_n);
    memcpy(frame.data, data_n, FRAME_SIZE_BYTES);
    return 0;
}

uint8_t updateFrame(frame &frame, uint8_t mode_n, uint8_t header_n)
{
    frame.mode = mode_n;
    frame.header = header_n;
    return 0;
}

uint8_t packFrame(vector<uint8_t> &signal, frame &frame)
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
uint8_t packetFromFrame(vector<uint8_t> &packet, frame &frame)
{
    packFrame(packet, frame);
    packet.push_back(find_crc(packet)); // TODO: test if this is doing anything

    return 0;
}

uint8_t printFrame(frame &frame)
{
    char chunk[10];
    string out;
    snprintf(chunk, 10, "%d | %x |", frame.mode == DATA_MODE, frame.header);
    out = chunk;

    for (int i = 0; i < FRAME_SIZE_BYTES; i++)
    {
        snprintf(chunk, 10, " %x ", frame.data[i]);
        out += chunk;
    }

    cout << out << endl;

    return 0;
}

// returns expected crc
crc_t find_crc(string &packet) {
    crc_t crc = crc_init();
    crc = crc_update(crc, packet.data(), packet.size());
    crc = crc_finalize(crc);
    return crc;
}

// returns expected crc
crc_t find_crc(vector<uint8_t> &packet) {
    crc_t crc = crc_init();
    crc = crc_update(crc, packet.data(), packet.size());
    crc = crc_finalize(crc);
    return crc;
}

uint8_t check_received_crc(string packet) {
    crc_t expected = (crc_t)packet[packet.size()-1];
    packet.pop_back();
    crc_t actual = find_crc(packet);

    return actual == expected;
}