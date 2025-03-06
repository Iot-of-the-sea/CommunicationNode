
#ifndef __AUDIO_TX__
#define __AUDIO_TX__

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
    explicit AudioTransmitter(const AudioProfile &profile);

    // Function to play sequence
    void play_sequence(std::vector<uint8_t> &sequence, bool preamble = false);

    // Function to generate a bit sequence waveform
    // little endian
    std::vector<double> generate_sequence(std::vector<uint8_t> &bytes, bool preamble = false);

    // Generate a sine wave of a given frequency
    std::vector<double> generate_frequency(double freq, double start = 0);

    // Generate low-frequency wave
    std::vector<double> generate_low(double start);

    // Generate high-frequency wave
    std::vector<double> generate_high(double start);

    // Function to play the generated waveform using PortAudio
    void play_audio(const std::vector<double> &signal);
};

#endif // __AUDIO_TX__