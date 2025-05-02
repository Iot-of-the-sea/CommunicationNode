
#ifndef __FSM__
#define __FSM__

#include "../lib/config.h"

#include <iostream>
#include <memory>
#include <thread>
#include <string>

#include "../lib/file_transfer/file_transfer.h"
#include "../lib/audio/audiotransmitter.h"
#include "../lib//audio/audioprofile.h"
#include "../lib/protocol.h"
#include "../lib/control.h"
#include <fstream>
#include <unistd.h>
#include <string.h>

#if DEPLOYED
#include "../lib/audio/audiorx/audioreceiver.h"
#else
#include "../tst/testlib/audioreceiver_test.h"
#endif

#if LINUX
#include "pa_linux_alsa.h"
#endif

using namespace std;

class NodeFSM;
class IdleState;

// Abstract base class for all states
class NodeState
{
public:
    virtual void handle(NodeFSM &fsm) = 0; // Pure virtual function
    virtual ~NodeState() = default;
};

class SendCtrlState : public NodeState
{
private:
    uint8_t _transmit_code;
    uint8_t _expected_receive;
    std::unique_ptr<NodeState> _nextState;
    std::unique_ptr<NodeState> _failState;
    uint32_t _timeout_us;
    uint16_t _maxTries;

public:
    SendCtrlState(uint8_t transmit_code, uint8_t expected_receive,
                  std::unique_ptr<NodeState> next, std::unique_ptr<NodeState> fail,
                  uint32_t timeout_us = 1000000, uint16_t maxTries = 10)
        : _transmit_code(transmit_code), _expected_receive(expected_receive), _nextState(move(next)),
          _failState(move(fail)), _timeout_us(timeout_us), _maxTries(maxTries) {};

    void handle(NodeFSM &fsm) override;
};

// Concrete states
class IdleState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

// Abstract base class for node FSMs
class NodeFSM
{
private:
    std::unique_ptr<NodeState> _state;
    bool _rov_mode;
    uint16_t _counter;

public:
    NodeFSM() : _state(std::make_unique<IdleState>()), _rov_mode(true),
                _counter(0) {}

    NodeFSM(bool rov_mode) : _state(std::make_unique<IdleState>()),
                             _rov_mode(rov_mode), _counter(0)
    {
        cout << "selected mode: " << _rov_mode << endl;
    }

    void changeState(std::unique_ptr<NodeState> newState)
    {
        _counter = 0;
        _state = std::move(newState);
    }

    void update()
    {
        _state->handle(*this);
    }

    bool getIsROVMode()
    {
        return _rov_mode;
    }

    uint16_t getCount() { return _counter; }
    void incrCount() { _counter++; }
};

class CalibrateState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class SearchState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class SendIDState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

// class SendRTSState : public NodeState
// {
// public:
//     void handle(NodeFSM &fsm) override;
// };

class SendRTSState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class SendHeaderState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class SendDataStartState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class SendDataFrameState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class SendDataDoneState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class SendEOTState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class EchoConfirmationState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadIDState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadConfirmationState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadRTSState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadHeaderState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadDataStartState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadDataFrameState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadEOTState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

int runFSM(bool rovMode);

#endif // __FSM__