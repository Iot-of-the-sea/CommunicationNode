#ifndef __AUDIO_RECEIVER__
#define __AUDIO_RECEIVER__

#include "AudioDevice.h"
#include "Sampling.h"
#include "PreambleDetector.h"
#include "Demodulation.h"
#include <thread>
#include <atomic>

#include <string.h>

using namespace std;

uint8_t init_receiver();

uint8_t listen(string &result);

uint8_t close_receiver();

#endif // __AUDIO_RECEIVER__