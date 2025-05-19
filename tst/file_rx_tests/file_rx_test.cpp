#include "../../lib/audio/audiotransmitter.h"
#include "../../lib/file_transfer/file_transfer.h"
#include <fstream>
#include <chrono>

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));

TimeoutHandler timeout(5000000);

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

    srand(time(0));
    string writeTarget = "./tst/test_received/test_rx_file_" + to_string(rand()) + ".txt";

    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();
    err = receiveFile_test(audioTx, writeTarget.c_str(), timeout, 5000, &rxTestData);
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
    cout << "Empty Packets          : " << rxTestData.empty_packets << endl;
    cout << "Failed CRCs            : " << rxTestData.crc_failed << endl;

    return 0;
}
