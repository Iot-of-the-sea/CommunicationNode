#include "AudioDevice.h"
#include "Sampling.h"
#include "PreambleDetector.h"
#include <thread>
#include <atomic>

#include <string.h>
using namespace std;

atomic<bool> preambleDetected(false);

thread samplingThread;
thread preambleThread;

snd_pcm_t *pcm_handle;
string received_str;

uint8_t init_receiver()
{
    pcm_handle = nullptr;
    if (!AudioDevice::init(pcm_handle))
    {
        return -1;
    }

    samplingThread = thread(samplingThreadFunc, pcm_handle);
    preambleThread = thread(run, received_str);

    return 0;
}

int listen()
{
    samplingThread.join();
    preambleThread.join();
}

uint8_t close()
{
    AudioDevice::close(pcm_handle);
    return 0;
}