#pragma once
#include <queue>
#include <vector>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <alsa/asoundlib.h>
#include "fft.h"

constexpr float INT24_MAX = 8388608.0f;
constexpr int BUFFER_SIZE_SAMPLING = 192;
constexpr int CHANNEL_COUNT = 4;

void samplingThreadFunc(snd_pcm_t* pcm_handle);

