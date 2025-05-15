#include "file_transfer.h"

uint8_t transmit_file(AudioTransmitter &tx, const char *fileName, TimeoutHandler &timeout, uint16_t maxTries)
{
    ifstream ifile(fileName, ifstream::binary); // Open the file
    if (!ifile)
    {
        cerr << "Error opening file!" << endl;
    }

    string response;
    vector<string> chunks;
    char *frameBuf = new char[FRAME_SIZE_BYTES]; // change to nonallocated
    uint8_t frameNum = 0;
    uint8_t err;

    uint8_t triesCount = 0;

    // TODO: thread these so that it make signals and plays at the same time
    while (ifile.read(frameBuf, FRAME_SIZE_BYTES)) // TODO: restructure this part for ack/nak
    {
        chunks.push_back(string(frameBuf, FRAME_SIZE_BYTES));
        memset(frameBuf, 0, FRAME_SIZE_BYTES);
    }
    chunks.push_back(string(frameBuf));

    string chunkStr;
    uint16_t chunkLen;
    while (frameNum < chunks.size() && triesCount < maxTries)
    {
        chunkStr = chunks.at(frameNum);
        chunkLen = chunkStr.length();
        memcpy(frameBuf, chunkStr.data(), chunkLen);
        cout << "transmit: " << (unsigned int)frameNum << endl;
        transmit_data(tx, DATA_MODE, frameNum, reinterpret_cast<uint8_t *>(frameBuf), chunkLen);

        err = listen(response, &timeout);

        if (!err && isAck(response))
        {
            cout << static_cast<unsigned int>(frameNum) << ": " << response.c_str()[0] << endl;
            frameNum++;
            triesCount = 0;
            cout << "ACK response" << endl;
        }
        else
        {
            triesCount++;
            cout << "NAK response" << endl;
        }
    }

    ifile.close();     // Close the file
    delete[] frameBuf; // TODO: check this

    return 0;
}

// TODO: test this
uint8_t receiveFile(AudioTransmitter &tx, const char *fileName, TimeoutHandler &timeout, uint16_t maxTries)
{
    string result, last_rx_data, rx_data;
    uint8_t headerByte = 0x00;
    uint8_t lastHeader = 0x00;
    uint8_t err;
    uint16_t counter = 0;

    FileWriter file(fileName);
    err = file.open();
    if (err)
    {
        cout << "error opening file" << endl;
        return err;
    }

    while (headerByte != DATA_DONE)
    {
        err = set_gpio_mode(RX_MODE);
        err = listen(result, &timeout);

        if (!err)
            err = getHeaderByte(result, headerByte);

        if (!err)
            err = get_packet_data(result, rx_data);

        if (!err && !check_received_crc(result))
            err = CRC_ERROR;

        set_gpio_mode(TX_MODE);
        if (!err)
        {
            cout << "NO ERROR" << endl;
            transmit_data(tx, CTRL_MODE, ACK);
            if (headerByte != lastHeader)
            {
                file.write(last_rx_data);
            }

            last_rx_data = rx_data;
            lastHeader = headerByte;
            counter = 0;
        }
        else
        {
            if (err == TIMEOUT_ERROR)
            {
                cout << "TIMEOUT" << endl;
            }
            else if (err == EMPTY_PACKET_ERROR)
            {
                cout << "EMPTY PACKET" << endl;
            }
            else if (err == CRC_ERROR)
            {
                cout << "CRC FAIL" << endl;
            }
            else
            {
                cout << "UNKNOWN ERROR" << endl;
            }

            transmit_data(tx, CTRL_MODE, NAK_SEND);
            if (err != CRC_ERROR)
                counter++;
        }

        if (counter >= maxTries)
        {
            err = TIMEOUT_ERROR;
            break;
        }
    }

    file.write(last_rx_data);
    file.close();

    return err;
}

#if PARAMETER_TESTING
uint8_t transmit_file_test(AudioTransmitter &tx, const char *file,
                           TimeoutHandler &timeout, uint16_t maxTries,
                           TxTestData *testData)
{
    testData->sent = 0;
    testData->ack = 0;
    testData->nak = 0;
    testData->timeouts = 0;

    ifstream ifile(file, ifstream::binary); // Open the file
    if (!ifile)
    {
        cerr << "Error opening file!" << endl;
    }

    string response;
    vector<string> chunks;
    char *frameBuf = new char[FRAME_SIZE_BYTES]; // change to nonallocated
    uint8_t frameNum = 0;
    uint8_t err;

    uint8_t timeoutCount = 0;

    // TODO: thread these so that it make signals and plays at the same time
    while (ifile.read(frameBuf, FRAME_SIZE_BYTES)) // TODO: restructure this part for ack/nak
    {
        chunks.push_back(string(frameBuf, FRAME_SIZE_BYTES));
        memset(frameBuf, 0, FRAME_SIZE_BYTES);
    }
    chunks.push_back(string(frameBuf));

    string chunkStr;
    uint16_t chunkLen;
    while (frameNum < chunks.size() && timeoutCount < maxTries)
    {
        chunkStr = chunks.at(frameNum);
        chunkLen = chunkStr.length();
        memcpy(frameBuf, chunkStr.data(), chunkLen);
        cout << "transmit: " << (unsigned int)frameNum << endl;

        set_gpio_mode(TX_MODE);
        transmit_data(tx, DATA_MODE, frameNum, reinterpret_cast<uint8_t *>(frameBuf), chunkLen);
        testData->sent++;

        set_gpio_mode(RX_MODE);
        err = listen(response, &timeout);
        if (err == TIMEOUT_ERROR)
            testData->timeouts++;
        else
        {
            if (isAck(response))
                testData->ack++;
            else
                testData->nak++;
        }

        if (err == TIMEOUT_ERROR)
        {
            timeoutCount++;
        }
        else
        {
            timeoutCount = 0;
        }

        if (!err && isAck(response))
        {
            cout << static_cast<unsigned int>(frameNum) << ": " << response.c_str()[0] << endl;
            frameNum++;
            cout << "ACK response" << endl;
        }
        else
        {
            cout << "NAK response" << endl;
        }
    }

    ifile.close();     // Close the file
    delete[] frameBuf; // TODO: check this

    return err;
}

// TODO: test this
uint8_t receiveFile_test(AudioTransmitter &tx, const char *fileName,
                         TimeoutHandler &timeout, uint16_t maxTries,
                         RxTestData *testData)
{
    testData->received = 0;
    testData->timeouts = 0;
    testData->empty_packets = 0;
    testData->crc_failed = 0;

    string result, last_rx_data, rx_data;
    uint8_t headerByte = 0x00;
    uint8_t lastHeader = 0x00;
    uint8_t err;
    uint16_t counter = 0;

    FileWriter file("./tst/testFile.txt");
    file.open();
    while (headerByte != DATA_DONE)
    {
        set_gpio_mode(RX_MODE);
        err = listen(result, &timeout);

        if (!err)
            err = getHeaderByte(result, headerByte);

        if (!err)
            err = get_packet_data(result, rx_data);

        if (!err && !check_received_crc(result))
            err = CRC_ERROR;

        set_gpio_mode(TX_MODE);
        if (!err)
        {
            cout << "NO ERROR" << endl;
            transmit_data(tx, CTRL_MODE, ACK);
            if (headerByte != lastHeader)
            {
                file.write(last_rx_data);
            }

            last_rx_data = rx_data;
            lastHeader = headerByte;
            counter = 0;
            testData->received++;
        }
        else
        {
            if (err == TIMEOUT_ERROR)
            {
                testData->timeouts++;
                cout << "TIMEOUT" << endl;
            }
            else if (err == EMPTY_PACKET_ERROR)
            {
                testData->received++;
                testData->empty_packets++;
                cout << "EMPTY PACKET" << endl;
            }
            else if (err == CRC_ERROR)
            {
                testData->received++;
                testData->crc_failed++;
                cout << "CRC FAIL" << endl;
            }
            else
            {
                testData->received++;
                cout << "UNKNOWN ERROR" << endl;
            }

            transmit_data(tx, CTRL_MODE, NAK_SEND);
            if (err != CRC_ERROR)
                counter++;
        }

        if (counter >= maxTries)
        {
            break;
        }
    }

    file.write(last_rx_data);
    file.close();

    return NO_ERROR;
}
#endif