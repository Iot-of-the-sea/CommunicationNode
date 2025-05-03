
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

class SendState : public NodeState
{
private:
    uint8_t _transmit_code;
    uint8_t _expected_receive;
    uint8_t _mode;
    std::unique_ptr<NodeState> _nextState;
    std::unique_ptr<NodeState> _failState;
    uint32_t _timeout_us;
    uint16_t _maxTries;

public:
    SendState(uint8_t transmit_code, uint8_t expected_receive, uint8_t mode,
              std::unique_ptr<NodeState> next, std::unique_ptr<NodeState> fail,
              uint32_t timeout_us = 1000000, uint16_t maxTries = 10)
        : _transmit_code(transmit_code), _expected_receive(expected_receive), _mode(mode),
          _nextState(move(next)), _failState(move(fail)), _timeout_us(timeout_us),
          _maxTries(maxTries) {};

    void handle(NodeFSM &fsm) override;
};

class ReadState : public NodeState
{
private:
    uint8_t _expected_receive;
    uint8_t _transmit_code;
    std::unique_ptr<NodeState> _nextState;
    std::unique_ptr<NodeState> _failState;
    bool _send_nak;
    uint32_t _timeout_us;

public:
    ReadState(uint8_t expected_receive, uint8_t transmit_code,
              std::unique_ptr<NodeState> next, std::unique_ptr<NodeState> fail,
              bool send_nak = true, uint32_t timeout_us = 2500000)
        : _expected_receive(expected_receive), _transmit_code(transmit_code),
          _nextState(move(next)), _failState(move(fail)), _send_nak(send_nak),
          _timeout_us(timeout_us) {};

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
        cout << "new state" << endl;
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

unique_ptr<NodeState> createSendIDState();

unique_ptr<NodeState> createSendRTSState();

class SendHeaderState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

unique_ptr<NodeState> createSendDataStartState();

class SendDataFrameState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

unique_ptr<NodeState> createSendDataDoneState();

unique_ptr<NodeState> createSendEOTState();
// class SendEOTState : public NodeState
// {
// public:
//     void handle(NodeFSM &fsm) override;
// };

class EchoConfirmationState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

unique_ptr<NodeState> createReadIDState();

unique_ptr<NodeState> createReadRTSState();

class ReadConfirmationState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReadHeaderState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

unique_ptr<NodeState> createReadDataStartState();

class ReadDataFrameState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

unique_ptr<NodeState> createReadEOTState();
// class ReadEOTState : public NodeState
// {
// public:
//     void handle(NodeFSM &fsm) override;
// };

int runFSM(bool rovMode);

#endif // __FSM__