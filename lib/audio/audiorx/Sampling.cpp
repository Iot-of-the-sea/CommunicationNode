#include "Sampling.h"
#include "PreambleDetector.h"
#include "Demodulation.h"

bool samplingRunning;

void samplingThreadFunc(snd_pcm_t *pcm_handle)
{
    int32_t rawSamples[BUFFER_SIZE_SAMPLING * CHANNEL_COUNT];
    samplingRunning = true;

    while (samplingRunning)
    {
        snd_pcm_sframes_t frames = snd_pcm_readi(pcm_handle, rawSamples, BUFFER_SIZE_SAMPLING);

        if (frames < 0)
        {
            snd_pcm_prepare(pcm_handle);
            continue;
        }

        vector<float> frameData;
        frameData.reserve(BUFFER_SIZE_SAMPLING);

        for (int i = 0; i < frames; ++i)
        {
            int32_t sample = rawSamples[i * CHANNEL_COUNT + 1]; // Take the first channel
            // int32_t sample = rawSamples[i * CHANNEL_COUNT + 2];  // Take the Loopback channel
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

// Stop the demodulation thread and clean up resources
void stopSampling()
{
    samplingRunning = false;
}