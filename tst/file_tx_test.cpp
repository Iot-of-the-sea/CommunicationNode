#include "../lib/audio/audiotransmitter.h"

AudioTransmitter audioTx(AudioProfile(1000.0, {240, 488}, 70000));

int main()
{
    audioTx.init_stream();
    cout << "stream initialized" << endl;
    transmit_file(audioTx, "./tst/test.txt");
    // transmit_data(audioTx, CTRL_MODE, RTS);
    audioTx.close_stream();
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
