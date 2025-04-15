#include "audiotransmitter.h"

AudioTransmitter::AudioTransmitter(const AudioProfile &profile) : audio(profile) {}

// Function to play sequence
void AudioTransmitter::play_sequence(std::vector<uint8_t> &sequence, bool preamble)
{
    auto signal = generate_sequence(sequence, preamble);
    play_audio(signal);
}

// Function to generate a bit sequence waveform
// little endian
std::vector<double> AudioTransmitter::generate_sequence(std::vector<uint8_t> &bytes, bool preamble)
{
    int byte_num = bytes.size();
    int bit_num = byte_num * 8;
    if (preamble)
        bit_num += 6;
    double seq_dur = bit_num * audio.get_bit_time();

    int sample_count = static_cast<int>(seq_dur * audio.get_sample_rate());

    std::vector<double> t(sample_count);
    std::vector<double> y;

    for (int i = 0; i < sample_count; ++i)
        t[i] = i / audio.get_sample_rate();

    std::vector<double> bit_wave;
    double offset = 0;
    if (preamble)
    {
        uint8_t preamble_bits[6] = {0, 0, 1, 1, 0, 0};
        for (size_t i = 0; i < 6; i++)
        {
            bit_wave = (preamble_bits[i]) ? generate_high(offset) : generate_low(offset);
            y.insert(y.end(), bit_wave.begin(), bit_wave.end());
            offset += audio.get_bit_time();
        }
    }

    // Generate waveforms
    for (size_t n = 0; n < byte_num; n++)
    {
        for (size_t m = 0; m < 8; m++)
        {
            bit_wave = (bytes[n] & (1 << m)) ? generate_high(offset) : generate_low(offset);
            y.insert(y.end(), bit_wave.begin(), bit_wave.end());
            offset += audio.get_bit_time();
        }
    }

    return y;
}

// Generate a sine wave of a given frequency
std::vector<double> AudioTransmitter::generate_frequency(double freq, double start)
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
std::vector<double> AudioTransmitter::generate_low(double start)
{
    return generate_frequency(audio.get_low(), start);
}

// Generate high-frequency wave
std::vector<double> AudioTransmitter::generate_high(double start)
{
    return generate_frequency(audio.get_high(), start);
}

// Function to play the generated waveform using PortAudio
void AudioTransmitter::play_audio(const std::vector<double> &signal)
{
    std::vector<float_t> pcm_signal(signal.size());
    int counter = 0;
    // Convert double samples to 16-bit PCM and write to file
    for (double sample : signal)
    {
        float_t pcm_sample = static_cast<float_t>(sample); // Convert to 16-bit PCM
        pcm_signal.at(counter) = pcm_sample;
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

uint8_t transmit_data(AudioTransmitter &tx, uint8_t mode, uint8_t header)
{
    uint8_t err;

    frame tx_frame = {
        .mode = mode,
        .header = header,
        .data = {}};

    std::vector<uint8_t> packet;

    err = packetFromFrame(packet, tx_frame);
    tx.play_sequence(packet, true);
    return err;
}

// TODO: simplify these two functions
uint8_t transmit_data(AudioTransmitter &tx, uint8_t mode, uint8_t header, uint8_t *data_n)
{
    uint8_t err;

    frame tx_frame = {
        .mode = mode,
        .header = header,
        .data = {}};

    std::memcpy(tx_frame.data, data_n, FRAME_SIZE_BYTES);
    std::vector<uint8_t> packet;

    err = packetFromFrame(packet, tx_frame);
    tx.play_sequence(packet, true);
    return err;
    return 0;
}

// // Main function to run the example
// int test()
// {
//     try
//     {
//         std::vector<uint8_t> single_sequence = {0b00101000, 0b00010001, 0b10001001, 0b11101000,
//                                                 0b00010101, 0b01010101, 0b00100010, 0b11101010};

//         std::vector<uint8_t> sequence;
//         for (int i = 0; i < 3; i++)
//         {
//             sequence.insert(sequence.end(), single_sequence.begin(), single_sequence.end());
//         }

//         std::vector<uint8_t> bits(sequence.begin(), sequence.end());

//         AudioProfile ap(500000.0, {120, 244}); // 1000 μs bit time, low 120 Hz, high 244 Hz
//         AudioTransmitter tx(ap);

//         tx.play_sequence(bits, true);
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << '\n';
//     }

//     return 0;
// }
