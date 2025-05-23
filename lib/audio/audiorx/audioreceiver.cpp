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

    return 0;
}

uint8_t listen(string &result)
{
    TimeoutHandler *nullTimeout = nullptr;
    return listen(result, nullTimeout);
}

uint8_t listen(string &result, TimeoutHandler *timeout)
{
    samplingThread = thread(samplingThreadFunc, pcm_handle);
    preambleThread = thread(run, std::ref(received_str), timeout);
    samplingThread.join();
    preambleThread.join();

    stopDemodulation();
    result = received_str;

    if (timeout->getTriggered()) // TODO: add function to get and clear in one
    {
        return TIMEOUT_ERROR;
    }
    return NO_ERROR;
}
uint8_t close_receiver()
{
    AudioDevice::close(pcm_handle);
    return 0;
}