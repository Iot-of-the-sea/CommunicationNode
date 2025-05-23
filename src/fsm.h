
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
#include <functional>

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
class InitState;

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
    function<unique_ptr<NodeState>()> _nextStateFactory;
    function<unique_ptr<NodeState>()> _failStateFactory;
    uint32_t _timeout_us;
    uint16_t _maxTries;

public:
    SendState(uint8_t transmit_code, uint8_t expected_receive, uint8_t mode,
              function<unique_ptr<NodeState>()> nextFactory,
              function<unique_ptr<NodeState>()> failFactory,
              uint32_t timeout_us = 1000000, uint16_t maxTries = 10)
        : _transmit_code(transmit_code), _expected_receive(expected_receive), _mode(mode),
          _nextStateFactory(move(nextFactory)), _failStateFactory(move(failFactory)),
          _timeout_us(timeout_us), _maxTries(maxTries) {};

    void handle(NodeFSM &fsm) override;
};

class ReadState : public NodeState
{
private:
    uint8_t _expected_receive;
    uint8_t _transmit_code;
    std::function<std::unique_ptr<NodeState>()> _nextStateFactory;
    std::function<std::unique_ptr<NodeState>()> _failStateFactory;
    bool _send_nak;
    uint32_t _timeout_us;

public:
    ReadState(uint8_t expected_receive, uint8_t transmit_code,
              function<unique_ptr<NodeState>()> nextFactory,
              function<unique_ptr<NodeState>()> failFactory,
              bool send_nak = true, uint32_t timeout_us = 2500000)
        : _expected_receive(expected_receive), _transmit_code(transmit_code),
          _nextStateFactory(move(nextFactory)), _failStateFactory(move(failFactory)),
          _send_nak(send_nak), _timeout_us(timeout_us) {};

    void handle(NodeFSM &fsm) override;
};

// Concrete states
class InitState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class DoneState : public NodeState
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
    const char *_tx_file;
    bool _unsent; // if there are any unsent files
    uint32_t _node_id;

public:
    NodeFSM() : _state(std::make_unique<InitState>()), _rov_mode(true),
                _counter(0), _tx_file(""), _unsent(false), _node_id(0) {}

    NodeFSM(bool rov_mode, const char *txFile) : _state(std::make_unique<InitState>()),
                                                 _rov_mode(rov_mode), _counter(0),
                                                 _tx_file(txFile), _node_id(0)
    {
        _unsent = _tx_file && _tx_file[0] != '\0';

        cout << "Mode: " << (_rov_mode ? "ROV" : "SENSOR") << endl;
        if (_tx_file)
            cout << "File: " << _tx_file << endl;
    }

    void changeState(std::unique_ptr<NodeState> newState)
    {
        cout << "NEW STATE" << endl;
        _counter = 0;
        if (newState == nullptr)
        {
            cerr << "nullptr" << endl;
        }
        _state = std::move(newState);
    }

    void update() { _state->handle(*this); }

    void setUnsent(bool unsent_n) { _unsent = unsent_n; }
    void setNodeID(uint32_t newID) { _node_id = newID; }

    bool getIsROVMode() { return _rov_mode; }
    bool getUnsent() { return _unsent; }
    uint16_t getCount() { return _counter; }

    const char *getFileName() { return _tx_file; }
    void incrCount() { _counter++; }
};

class IdleState : public NodeState
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

class ReadIDState : public NodeState
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

class TransmitDoneState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

class ReceiveDoneState : public NodeState
{
public:
    void handle(NodeFSM &fsm) override;
};

int runFSM(bool rovMode, const char *txFile);

#endif // __FSM__