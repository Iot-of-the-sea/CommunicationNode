#include "../../lib/file_transfer/file_transfer.h"
#include <chrono>

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));
TimeoutHandler timeout(500000);

TxTestData txTestData;

int main()
{
    cout << "running file Tx tests" << endl;
    audioTx.init_stream();
    init_receiver();
    init_gpio();
    init_pins("toggle");

    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();
    transmit_file_test(audioTx, "./tst/test.txt", timeout, &txTestData);
    transmit_data(audioTx, CTRL_MODE, DATA_DONE);
    chrono::steady_clock::time_point endTime = chrono::steady_clock::now();

    string result;
    // uint8_t err = listen(result, &timeout);
    // if (err)
    // {
    //     transmit_data(audioTx, CTRL_MODE, DATA_DONE);
    //     err = listen(result, &timeout);
    // }
    audioTx.close_stream();
    close_receiver();
    close_gpio();

    cout << "---------- RESULTS ----------" << endl;
    cout << "Elapsed Time       : "
         << chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()
         << " ms" << endl;
    cout << "Total Packets Sent : " << txTestData.sent << endl;
    cout << "ACKs Received      : " << txTestData.ack << endl;
    cout << "NAKs Recevied      : " << txTestData.nak << endl;
    cout << "Timeouts           : " << txTestData.timeouts << endl;
}

// Main function to run the example
int test_tx()
{
    try
    {
        std::vector<uint8_t> single_sequence = {0b00101000, 0b00010001, 0b10001001, 0b11101000,
                                                0b00010101, 0b01010101, 0b00100010, 0b11101010};

        std::vector<uint8_t> sequence;
        for (int i = 0; i < 3; i++)
        {
            sequence.insert(sequence.end(), single_sequence.begin(), single_sequence.end());
        }

        std::vector<uint8_t> bits(sequence.begin(), sequence.end());

        AudioProfile ap(500000.0, {120, 244}, 500); // 1000 μs bit time, low 120 Hz, high 244 Hz
        AudioTransmitter tx(ap);

        tx.play_sequence(bits, true);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}

// Example usage
int test_profile()
{
    try
    {
        std::vector<double> freq_list(2);
        freq_list[0] = 1000;
        freq_list[1] = 2000;
        AudioProfile profile(500, freq_list, 3000); // 500 μs bit time

        profile.print_info();

        // Change sample rate
        profile.set_sample_rate(44100);
        std::cout << "Updated Sample Rate: " << profile.get_sample_rate() << " Hz\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}
