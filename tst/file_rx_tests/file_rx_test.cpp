#include "../../lib/audio/audiotransmitter.h"
#include <fstream>

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));

TimeoutHandler timeout(1000000);

string result, last_rx_data, rx_data;
uint8_t headerByte = 0x00;
uint8_t lastHeader = 0x00;
uint8_t err;

int main()
{
    cout << "running file Rx tests" << endl;
    audioTx.init_stream();
    init_receiver();
    FileWriter file("./tst/testFile.txt");
    file.open();
    int counter = 0;
    while (headerByte != DATA_DONE && counter < 20)
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
            transmit_data(audioTx, CTRL_MODE, ACK);
            counter = 0;
        }
        else
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            counter++;
        }
    }

    audioTx.close_stream();
    close_receiver();
    file.close();

    cout << "close" << endl;

    return 0;
}
