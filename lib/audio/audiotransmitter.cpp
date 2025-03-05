#include <iostream>
#include <vector>
#include <portaudio.h>
#include "audioprofile.h" // Include your AudioProfile class

class AudioTransmitter
{
private:
    AudioProfile audio;

public:
    // Constructor
    explicit AudioTransmitter(const AudioProfile &profile) : audio(profile) {}

    // Function to play sequence
    void play_sequence(std::vector<uint8_t> &sequence)
    {
        auto signal = generate_sequence(sequence);
        play_audio(signal);
    }

    // Function to generate a bit sequence waveform
    std::vector<double> generate_sequence(std::vector<uint8_t> &bits)
    {
        for (auto &bit : bits)
            bit -= '0'; // Convert ASCII '0'/'1' to integer

        if (!std::all_of(bits.begin(), bits.end(), [](uint8_t b)
                         { return b == 0 || b == 1; }))
        {
            throw std::invalid_argument("Sequence must contain only '0' and '1'.");
        }

        int bit_num = bits.size();
        double seq_dur = bit_num * audio.get_bit_time();
        int sample_count = static_cast<int>(seq_dur * audio.get_sample_rate());

        std::vector<double> t(sample_count);
        std::vector<double> y;

        for (int i = 0; i < sample_count; ++i)
            t[i] = i / audio.get_sample_rate();

        // Generate waveforms
        for (size_t n = 0; n < bits.size(); ++n)
        {
            double bit_start = n * audio.get_bit_time();
            std::vector<double> bit_wave = (bits[n]) ? generate_high(bit_start) : generate_low(bit_start);
            y.insert(y.end(), bit_wave.begin(), bit_wave.end());
        }

        return y;
    }

    // Generate a sine wave of a given frequency
    std::vector<double> generate_frequency(double freq, double start = 0)
    {
        double bit_time = audio.get_bit_time();
        int sample_count = static_cast<int>(bit_time * audio.get_sample_rate());

        std::vector<double> y(sample_count);
        double dt = 1.0 / audio.get_sample_rate(); // Time step

        double t;
        for (int i = 0; i < sample_count; ++i)
        {
            t = (i * dt) + start;
            y[i] = std::sin(2 * M_PI * freq * t) * audio.get_amplitude();
        }
        return y;
    }

    // Generate low-frequency wave
    std::vector<double> generate_low(double start)
    {
        return generate_frequency(audio.get_low(), start);
    }

    // Generate high-frequency wave
    std::vector<double> generate_high(double start)
    {
        return generate_frequency(audio.get_high(), start);
    }

    // Function to play the generated waveform using PortAudio
    void play_audio(const std::vector<double> &signal)
    {
        std::vector<float_t> pcm_signal(signal.size());
        int counter = 0;
        // Convert double samples to 16-bit PCM and write to file
        for (double sample : signal)
        {
            float_t pcm_sample = static_cast<float_t>(sample); // Convert to 16-bit PCM
            pcm_signal[counter] = pcm_sample;
            counter++;
        }

        Pa_Initialize();
        PaStream *stream;
        Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, audio.get_sample_rate(), paFramesPerBufferUnspecified, nullptr, nullptr);
        Pa_StartStream(stream);
        Pa_WriteStream(stream, pcm_signal.data(), pcm_signal.size());
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
    }
};

// // Main function to run the example
// int main()
// {
//     try
//     {
//         std::string sequence =
//             "11101010001000100101010100010101111010001000100100010001001010";
//         sequence = sequence + sequence + sequence + sequence + sequence; // Repeat 30x (simplified)
//         sequence = sequence + sequence + sequence + sequence + sequence + sequence;

//         std::vector<uint8_t> bits(sequence.begin(), sequence.end());

//         AudioProfile ap(1000.0, {120, 244}); // 1000 μs bit time, low 120 Hz, high 244 Hz
//         AudioTransmitter tx(ap);

//         tx.play_sequence(bits);
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << '\n';
//     }

//     return 0;
// }
