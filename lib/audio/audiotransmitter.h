
#ifndef __AUDIO_TX__
#define __AUDIO_TX__

#include <iostream>
#include <vector>
#include <portaudio.h>
#include <stdlib.h>
#include "audioprofile.h" // Include your AudioProfile class

#include "../data.h"
#include "../control.h"

#define DEPLOYED true
#define LINUX true

#if DEPLOYED
#include "./audiorx/audioreceiver.h"
#else
#include "../../tst/testlib/audioreceiver_test.h"
#endif

#if LINUX
#include "pa_linux_alsa.h"
#endif

using namespace std;

class AudioTransmitter
{
private:
    AudioProfile audio;
    PaStream *stream;
    uint8_t stream_status;

public:
    // Constructor
    explicit AudioTransmitter(const AudioProfile &profile);

    uint8_t init_stream();

    uint8_t close_stream();

    // Function to play sequence
    void play_sequence(vector<uint8_t> &sequence, bool preamble = false);

    // Function to generate a bit sequence waveform
    // little endian
    vector<double> generate_sequence(vector<uint8_t> &bytes, bool preamble = false);

    // Generate a sine wave of a given frequency
    vector<double> generate_frequency(double freq, double start = 0);

    // Generate low-frequency wave
    vector<double> generate_low(double start);

    // Generate high-frequency wave
    vector<double> generate_high(double start);

    // Generate stop-frequency wave
    vector<double> generate_stop(double start);

    // Function to play the generated waveform using PortAudio
    void play_audio(const vector<double> &signal);
};

uint8_t transmit_data(AudioTransmitter &tx, uint8_t mode, uint8_t header);

uint8_t transmit_data(AudioTransmitter &tx, uint8_t mode, uint8_t header, uint8_t *data_n, uint16_t len);

uint8_t transmit_file(AudioTransmitter &tx, const char *file);

#endif // __AUDIO_TX__