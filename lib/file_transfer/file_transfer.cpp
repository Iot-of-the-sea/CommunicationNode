#include "file_transfer.h"

// TODO: test this
uint8_t receiveFile(AudioTransmitter &tx, const char *fileName, TimeoutHandler &timeout, uint16_t maxTries)
{
    string result, last_rx_data, rx_data;
    uint8_t headerByte = 0x00;
    uint8_t lastHeader = 0x00;
    uint8_t err;
    uint16_t counter = 0;

    FileWriter file("./tst/testFile.txt");
    file.open();
    while (headerByte != DATA_DONE && counter < maxTries)
    {
        err = listen(result, &timeout);
        if (!err && check_received_crc(result))
        {
            getHeaderByte(result, headerByte);
            get_packet_data(result, rx_data);

            if (headerByte != lastHeader)
            {
                file.write(last_rx_data);
            }

            last_rx_data = rx_data;
            lastHeader = headerByte;
            transmit_data(tx, CTRL_MODE, ACK);
            counter = 0;
        }
        else
        {
            transmit_data(tx, CTRL_MODE, NAK_SEND);
            counter++;
        }
    }

    file.close();

    return NO_ERROR;
}
