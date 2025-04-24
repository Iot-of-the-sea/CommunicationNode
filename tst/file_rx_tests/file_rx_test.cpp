#include "../../lib/audio/audiotransmitter.h"

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));

string result, rx_data;
uint8_t headerByte = 0x00;
uint8_t err;

int main()
{
    audioTx.init_stream();
    init_receiver();
    // cout << "stream initialized" << endl;
    while (headerByte != DATA_DONE)
    {
        err = listen(result);
        usleep(1000);
        getHeaderByte(result, headerByte);
        get_packet_data(result, rx_data);

        if (check_received_crc(result))
        {
            transmit_data(audioTx, CTRL_MODE, ACK);
            cout << rx_data << endl;
        }
        else
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            cout << "bad attempt" << endl;
        }
    }
    audioTx.close_stream();
    close_receiver();
}
