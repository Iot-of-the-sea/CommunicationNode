
#ifndef __TIMEOUT__
#define __TIMEOUT__

#include <stdint.h>
#include <chrono>
#include "../control.h"

using namespace std;

#define NO_TIMEOUT 0x00

enum TimeoutState
{
    IDLE,
    ACTIVE
};

class TimeoutHandler
{
private:
    TimeoutState _state;
    uint32_t _duration_us;
    bool _triggered;
    chrono::steady_clock::time_point _start_time;

public:
    TimeoutHandler() : _state(IDLE), _duration_us(0), _triggered(false) {}
    TimeoutHandler(uint32_t dur_us) : _state(IDLE), _duration_us(dur_us), _triggered(false) {}

    uint8_t start();
    uint8_t setDuration(uint32_t dur_us);

    uint8_t startTimeout(uint32_t dur_us);

    uint8_t checkTimeout();

    TimeoutState peepState() { return _state; }
    uint32_t getDuration() { return _duration_us; }
    bool getTriggered() { return _triggered; }
    void clearTriggered() { _triggered = false; }

    uint8_t reset();
};

#endif // __TIMEOUT__