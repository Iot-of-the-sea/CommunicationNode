#include "audioreceiver.h"
#include <functional>

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
    preambleThread = thread(run, std::ref(received_str));

    return 0;
}

uint8_t listen(string &result)
{
    samplingThread.join();
    preambleThread.join();
    result = received_str;
    return 0;
}

uint8_t close_receiver()
{
    AudioDevice::close(pcm_handle);
    return 0;
}