#include "../../lib/audio/audiotransmitter.h"

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));

TimeoutHandler timeout(1000000);

string result, rx_data;
uint8_t headerByte = 0x00;
uint8_t err;

int main()
{
    cout << "running file Rx tests" << endl;
    audioTx.init_stream();
    init_receiver();
    while (headerByte != DATA_DONE)
    {
        err = listen(result, &timeout);
        if (!err && check_received_crc(result))
        {
            getHeaderByte(result, headerByte);
            get_packet_data(result, rx_data);
            transmit_data(audioTx, CTRL_MODE, ACK);
            cout << "data: " << rx_data << endl;
        }
        else
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            // cout << "bad attempt" << endl;
        }
    }
    transmit_data(audioTx, CTRL_MODE, ACK);

    cout << "not segfaulted 1" << endl;
    audioTx.close_stream();
    close_receiver();

    cout << "not segfaulted 2" << endl;

    return 0;
}
