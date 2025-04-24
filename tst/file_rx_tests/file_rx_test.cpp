#include "../../lib/audio/audiotransmitter.h"

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));

string result;
uint8_t headerByte = 0x00;
uint8_t err;

int main()
{
    audioTx.init_stream();
    init_receiver();
    cout << "stream initialized" << endl;
    while (headerByte != DATA_DONE)
    {
        err = listen(result);
        usleep(1000);
        transmit_data(audioTx, CTRL_MODE, ACK);
    }
    audioTx.close_stream();
    close_receiver();
}
