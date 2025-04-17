#include "audiotransmitter.h"

AudioTransmitter::AudioTransmitter(const AudioProfile &profile) : audio(profile), stream_status(0) {}

uint8_t AudioTransmitter::init_stream()
{
    Pa_Initialize();
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, audio.get_sample_rate(), paFramesPerBufferUnspecified, nullptr, nullptr);
    Pa_StartStream(stream);
    stream_status = 1;

    return 0;
}

uint8_t AudioTransmitter::close_stream()
{
    Pa_CloseStream(stream);
    Pa_Terminate();
    Pa_StopStream(stream);
    stream_status = 0;

    return 0;
}

// Function to play sequence
void AudioTransmitter::play_sequence(vector<uint8_t> &sequence, bool preamble)
{
    if (!stream_status)
    {
        cerr << "Error: Audio stream not initialized." << endl;
    }
    auto signal = generate_sequence(sequence, preamble);
    play_audio(signal);
}

// Function to generate a bit sequence waveform
// little endian
vector<double> AudioTransmitter::generate_sequence(vector<uint8_t> &bytes, bool preamble)
{
    int byte_num = bytes.size();
    int bit_num = byte_num * 8;
    if (preamble)
        bit_num += 8;
    double seq_dur = bit_num * audio.get_bit_time();

    int sample_count = static_cast<int>(seq_dur * audio.get_sample_rate());

    vector<double> t(sample_count);
    vector<double> y;

    for (int i = 0; i < sample_count; ++i)
        t[i] = i / audio.get_sample_rate();

    vector<double> bit_wave;
    double offset = 0;
    if (preamble)
    {
        uint8_t preamble_bits[8] = {0, 0, 1, 1, 0, 0, 1, 1}; // TODO: calculate preamble only once
        for (size_t i = 0; i < 8; i++)
        {
            bit_wave = (preamble_bits[i]) ? generate_high(offset) : generate_low(offset);
            y.insert(y.end(), bit_wave.begin(), bit_wave.end());
            offset += audio.get_bit_time();
        }
    }

    // Generate waveforms
    for (size_t n = 0; n < byte_num; n++)
    {
        for (uint8_t m = 0; m < 8; m++)
        {
            bit_wave = (bytes[n] & (1 << (7 - m))) ? generate_high(offset) : generate_low(offset);
            y.insert(y.end(), bit_wave.begin(), bit_wave.end());
            offset += audio.get_bit_time();
        }
    }

    // Generate stop signal
    bit_wave = generate_stop(offset);
    y.insert(y.end(), bit_wave.begin(), bit_wave.end());

    return y;
}

// Generate a sine wave of a given frequency
vector<double> AudioTransmitter::generate_frequency(double freq, double start)
{
    double bit_time = audio.get_bit_time();
    int sample_count = static_cast<int>(bit_time * audio.get_sample_rate());

    vector<double> y(sample_count);
    double dt = 1.0 / audio.get_sample_rate(); // Time step

    double t;
    for (int i = 0; i < sample_count; ++i)
    {
        t = (i * dt) + start;
        y[i] = sin(2 * M_PI * freq * t) * audio.get_amplitude();
    }
    return y;
}

// Generate low-frequency wave
vector<double> AudioTransmitter::generate_low(double start)
{
    return generate_frequency(audio.get_low(), start);
}

// Generate high-frequency wave
vector<double> AudioTransmitter::generate_high(double start)
{
    return generate_frequency(audio.get_high(), start);
}

// Generate high-frequency wave
vector<double> AudioTransmitter::generate_stop(double start)
{
    return generate_frequency(audio.get_stop(), start);
}

// Function to play the generated waveform using PortAudio
void AudioTransmitter::play_audio(const vector<double> &signal)
{
    vector<float_t> pcm_signal(signal.size());
    int counter = 0;
    // Convert double samples to 16-bit PCM and write to file
    for (double sample : signal)
    {
        float_t pcm_sample = static_cast<float_t>(sample); // Convert to 16-bit PCM
        pcm_signal.at(counter) = pcm_sample;
        counter++;
    }
    Pa_WriteStream(stream, pcm_signal.data(), pcm_signal.size());
}

uint8_t transmit_data(AudioTransmitter &tx, uint8_t mode, uint8_t header)
{
    uint8_t err;

    frame tx_frame = {
        .mode = mode,
        .header = header,
        .data = {0}};

    vector<uint8_t> packet;

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
        .data = {0}};

    memcpy(tx_frame.data, data_n, FRAME_SIZE_BYTES);
    vector<uint8_t> packet;

    err = packetFromFrame(packet, tx_frame);
    tx.play_sequence(packet, true);
    return err;
    return 0;
}

uint8_t transmit_file(AudioTransmitter &tx, const char *file)
{
    ifstream ifile(file, ifstream::binary); // Open the file
    if (!ifile)
    {
        cerr << "Error opening file!" << endl;
    }

    vector<string> chunks;
    char *frameBuf = new char[FRAME_SIZE_BYTES];
    uint8_t frameNum = 0;

    // TODO: thread these so that it make signals and plays at the same time
    while (ifile.read(frameBuf, FRAME_SIZE_BYTES)) // TODO: restructure this part for ack/nak
    {
        chunks.push_back(string(frameBuf, FRAME_SIZE_BYTES));
        frameBuf = new char[FRAME_SIZE_BYTES];
    }
    chunks.push_back(string(frameBuf, FRAME_SIZE_BYTES));

    while (frameNum < chunks.size())
    {
        frameBuf = chunks.at(frameNum).data();
        transmit_data(tx, DATA_MODE, frameNum, reinterpret_cast<uint8_t *>(frameBuf));
        frameNum++;
    }

    ifile.close(); // Close the file

    return 0;
}