#include "../../lib/audio/audiotransmitter.h"
#include "../../lib/file_transfer/file_transfer.h"
#include <fstream>

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));

TimeoutHandler timeout(50000);

string result, last_rx_data, rx_data;
uint8_t headerByte = 0x00;
uint8_t lastHeader = 0x00;
uint8_t err;

RxTestData rxTestData;

int main()
{
    cout << "running file Rx tests" << endl;
    audioTx.init_stream();
    init_receiver();
    init_gpio();
    init_pins("toggle");

    set_gpio_mode(RX_MODE);

    while (true)
    {
        cout << ".";
    }
    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();
    // err = receiveFile_test(audioTx, "./tst/testFile.txt", timeout, 20, &rxTestData);
    chrono::steady_clock::time_point endTime = chrono::steady_clock::now();

    audioTx.close_stream();
    close_receiver();
    close_gpio();

    cout << "---------- RESULTS ----------" << endl;
    cout << "Elapsed Time       : "
         << chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()
         << " ms" << endl;
    cout << "Total Packets Received : " << rxTestData.received << endl;
    cout << "Timeouts               : " << rxTestData.timeouts << endl;
    cout << "Failed CRCs            : " << rxTestData.crc_failed << endl;

    close_gpio();
    return 0;
}
