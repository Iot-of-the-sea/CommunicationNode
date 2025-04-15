
#ifndef __FSM__
#define __FSM__

#include <iostream>
#include <memory>
#include <thread>
#include <string>

#include "../lib/audio/audiotransmitter.h"
#include "../lib//audio/audioprofile.h"
#include "../lib/protocol.h"
#include "../lib/control.h"
#include <fstream>
#include <unistd.h>
#include <string.h>

#define DEPLOYED false

#if DEPLOYED
#include "../lib/audio/audiorx/audioreceiver.h"
#else
#include "../tst/testlib/audioreceiver_test.h"
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

public:
    NodeFSM() : _state(std::make_unique<IdleState>()), _rov_mode(true) {}

    NodeFSM(bool rov_mode) : _state(std::make_unique<IdleState>()), _rov_mode(rov_mode)
    {
        cout << _rov_mode << endl;
    }

    void changeState(std::unique_ptr<NodeState> newState)
    {
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

int runFSM();

#endif // __FSM__