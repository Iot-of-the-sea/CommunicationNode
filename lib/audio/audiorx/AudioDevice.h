#pragma once
#include <alsa/asoundlib.h>

class AudioDevice {
public:
    static bool init(snd_pcm_t*& pcm_handle);
    static void close(snd_pcm_t* pcm_handle);
};

