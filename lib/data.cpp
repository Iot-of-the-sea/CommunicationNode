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
        signal.resize(frame.data_len + 1);
        for (size_t byte = 0; byte < frame.data_len; byte++)
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

uint8_t get_packet_data(string &packet, string &data)
{
    if (packet.size() < 3)
        return EMPTY_PACKET_ERROR;

    data = packet.substr(1, packet.length() - 2);
    return 0;
}

uint8_t frameFromHeaderData(frame &frame, headerData &header)
{
    frame.mode = DATA_MODE;
    frame.header = HEADER_DATA;
    frame.data_len = 6;

    cout << "size: " << (unsigned int)header.fileSizeBytes << endl;
    cout << "id: " << (unsigned int)header.nodeId << endl;

    uint8_t headerDataBytes[FRAME_SIZE_BYTES] = {0};
    headerDataBytes[0] = (uint8_t)(header.nodeId);
    headerDataBytes[1] = (uint8_t)(header.nodeId >> 8);

    for (size_t i = 0; i < 4; i++)
    {
        headerDataBytes[2 + i] = (uint8_t)(header.fileSizeBytes >> (8 * i));
    }

    for (auto &i : headerDataBytes)
    {
        cout << (unsigned int)i << endl;
    }

    memcpy(frame.data, headerDataBytes, FRAME_SIZE_BYTES);
    return NO_ERROR;
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
crc_t find_crc(string &packet)
{
    crc_t crc = crc_init();
    crc = crc_update(crc, packet.data(), packet.size());
    crc = crc_finalize(crc);
    return crc;
}

// returns expected crc
crc_t find_crc(vector<uint8_t> &packet)
{
    crc_t crc = crc_init();
    crc = crc_update(crc, packet.data(), packet.size());
    crc = crc_finalize(crc);
    return crc;
}

uint8_t check_received_crc(string packet)
{
    crc_t expected = (crc_t)packet[packet.size() - 1];
    packet.pop_back();
    crc_t actual = find_crc(packet);

    return actual == expected;
}

FileWriter::FileWriter(string fileName)
{
    if (fileName.length() < 1)
    {
        cerr << "Error: Failed to open file. Invalid file name." << endl;
        return;
    }

    _fileName = fileName;
}

FileWriter::FileWriter(const char *fileName)
{
    if (strlen(fileName) < 1)
    {
        cerr << "Error: Failed to open file. Invalid file name." << endl;
        return;
    }

    _fileName = string(fileName);
}

uint8_t FileWriter::open()
{
    if (_fileName.empty())
    {
        cerr << "Error: File name not given." << endl;
        return ARGUMENT_ERROR;
    }

    if (!_writeStream.is_open())
    {
        // append and binary mode
        _writeStream = ofstream(_fileName, std::ios::app | ios::binary);
        if (!_writeStream)
        {
            cerr << "Error: Cannot open file for writing!" << endl;
            return IO_ERROR;
        }
    }

    return NO_ERROR;
}

uint8_t FileWriter::close()
{
    _writeStream.close(); // Close the file
    if (_writeStream.is_open())
    {
        cerr << "Failed to close file." << endl;
        return IO_ERROR;
    }

    return NO_ERROR;
}

uint8_t FileWriter::write(char *data, uint32_t length)
{
    if (!_writeStream.is_open())
    {
        cerr << "Error: File not opened for writing." << endl;
        return IO_ERROR;
    }

    _writeStream.write(data, length); // Write header
    return NO_ERROR;
}

uint8_t FileWriter::write(char *data)
{
    return write(data, strlen(data));
}

uint8_t FileWriter::write(string data)
{
    return write(data.data(), data.length());
}

uint8_t FileWriter::write(uint8_t *data, uint32_t length)
{
    return write(reinterpret_cast<char *>(data), length);
}
