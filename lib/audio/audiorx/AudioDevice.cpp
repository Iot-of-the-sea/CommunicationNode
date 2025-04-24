#include "AudioDevice.h"
#include <iostream>

constexpr int SAMPLE_RATE = 192000;
constexpr int CHANNEL_COUNT = 2;
constexpr const char *DEVICE_NAME = "hw:2,0";

bool AudioDevice::init(snd_pcm_t *&pcm_handle)
{
    snd_pcm_hw_params_t *params;
    unsigned int rate = SAMPLE_RATE;
    int err;

    if ((err = snd_pcm_open(&pcm_handle, DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        std::cerr << "Unable to open device: " << snd_strerror(err) << std::endl;
        return false;
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S32_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNEL_COUNT);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, nullptr);

    snd_pcm_uframes_t period_size = 192;
    snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &period_size, nullptr);

    snd_pcm_uframes_t buffer_size = period_size * 10; // 10ms buffer
    snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size);

    if ((err = snd_pcm_hw_params(pcm_handle, params)) < 0)
    {
        std::cerr << "Failed to set hardware parameters: " << snd_strerror(err) << std::endl;
        return false;
    }

    return true;
}

void AudioDevice::close(snd_pcm_t *pcm_handle)
{
    snd_pcm_close(pcm_handle);
}
