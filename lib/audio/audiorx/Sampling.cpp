#include "Sampling.h"
#include "PreambleDetector.h"
#include "Demodulation.h"

void samplingThreadFunc(snd_pcm_t *pcm_handle)
{
    int32_t rawSamples[BUFFER_SIZE_SAMPLING * CHANNEL_COUNT];

    while (true)
    {
        snd_pcm_sframes_t frames = snd_pcm_readi(pcm_handle, rawSamples, BUFFER_SIZE_SAMPLING);

        if (frames < 0)
        {
            cout << "ERROR: " << snd_strerror(frames) << endl;
            snd_pcm_prepare(pcm_handle);
            continue;
        }

        vector<float> frameData;
        frameData.reserve(BUFFER_SIZE_SAMPLING);

        for (int i = 0; i < frames; ++i)
        {
            float_t sample_a = rawSamples[i * CHANNEL_COUNT];     // Take the first channel
            float_t sample = rawSamples[i * CHANNEL_COUNT + 1];   // Take the second channel
            float_t sample_b = rawSamples[i * CHANNEL_COUNT + 2]; // Take the Loopback channel
            float_t sample_c = rawSamples[i * CHANNEL_COUNT + 3]; // Take the Loopback channel
            float_t sample_d = rawSamples[i * CHANNEL_COUNT + 4]; // Take the Loopback channel
            float_t sample_e = rawSamples[i * CHANNEL_COUNT + 5]; // Take the Loopback channel
            float_t sample_f = rawSamples[i * CHANNEL_COUNT + 6]; // Take the Loopback channel
            float_t sample_g = rawSamples[i * CHANNEL_COUNT + 7]; // Take the Loopback channel

            cout << sample_a << " ; " << sample << " ; " << sample_b << " ; " << sample_c << " ; " << sample_d << " ; ";
            cout << sample_e << " ; " << sample_f << " ; " << sample_g << " ; " << endl;

            float normalized = (static_cast<float>(sample) / 256) / INT24_MAX;
            frameData.push_back(normalized);
        }

        // Route the data based on the current system state:
        // If Demodulation is already active, send the data to the Demodulation module;
        // Otherwise, update the buffer for preamble detection.
        if (isDemodulationActive())
        {
            addSamplingData(frameData);
            // std::cout << "add data to demod" << std::endl;
        }
        else
        {
            updateBuffer(frameData);
            // std::cout << "add data to preamble detection" << std::endl;
        }
    }
}
