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
    // little endian
    std::vector<double> generate_sequence(std::vector<uint8_t> &bytes)
    {
        int byte_num = bytes.size();
        double seq_dur = byte_num * 8 * audio.get_bit_time();
        int sample_count = static_cast<int>(seq_dur * audio.get_sample_rate());

        std::vector<double> t(sample_count);
        std::vector<double> y;

        for (int i = 0; i < sample_count; ++i)
            t[i] = i / audio.get_sample_rate();

        // Generate waveforms
        for (size_t n = 0; n < byte_num; n++)
        {
            for (size_t m = 0; m < 8; m++)
            {
                int i = n * 8 + m;
                double bit_start = i * audio.get_bit_time();
                std::vector<double> bit_wave = (bytes[n] & (1 << m)) ? generate_high(bit_start) : generate_low(bit_start);
                y.insert(y.end(), bit_wave.begin(), bit_wave.end());
            }
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

// Main function to run the example
int test()
{
    try
    {
        std::vector<uint8_t> single_sequence = {0b00101000, 0b00010001, 0b10001001, 0b11101000,
                                                0b00010101, 0b01010101, 0b00100010, 0b11101010};

        std::vector<uint8_t> sequence;
        for (int i = 0; i < 30; i++)
        {
            sequence.insert(sequence.end(), single_sequence.begin(), single_sequence.end());
        }

        std::vector<uint8_t> bits(sequence.begin(), sequence.end());

        AudioProfile ap(1000.0, {120, 244}); // 1000 μs bit time, low 120 Hz, high 244 Hz
        AudioTransmitter tx(ap);

        tx.play_sequence(bits);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}
