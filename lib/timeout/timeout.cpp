#include "timeout.h"

uint8_t TimeoutHandler::start()
{
    _start_time = chrono::steady_clock::now();
    _state = ACTIVE;
    clearTriggered();
    return NO_ERROR;
}

uint8_t TimeoutHandler::setDuration(uint32_t dur_us)
{
    if (_state != IDLE)
        return STATE_ERROR;

    _duration_us = dur_us;
    return NO_ERROR;
}

uint8_t TimeoutHandler::checkTimeout()
{
    if (_state == ACTIVE)
    {
        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::microseconds>(now - _start_time).count();
        if (elapsed >= _duration_us)
        {
            _state = IDLE;
            _triggered = true;
            return TIMEOUT_ERROR;
        }
    }
    return NO_TIMEOUT;
}

uint8_t TimeoutHandler::startTimeout(uint32_t dur_us)
{
    uint8_t err;
    err = setDuration(dur_us);

    if (!err)
        err = start();

    return err;
}