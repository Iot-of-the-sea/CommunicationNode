#include "fsm.h"

/***
 * TODO:
 * 1. Implement DoneState - DONE
 * 2. Fix ReadHeader - GOOD FOR NOW
 * 3. Add back failure case for transmitting - GOOD
 * 4. Fix failure transitions - GOOD ENOUGH
 * 5. Clean/pare down FSM states - GOOD
 * 6. Add EOT response for all states - NEVERMIND
 * 7. Generalize file management - GOOD ENOUGH FOR NOW (but still have to fix)
 * 8. Fix no file issue - DONE
 * 9. Implement 2-way file transfer
 * 10. Improve read ID to be more generalized
 * 11. Improve header/metadata stuff
 *     - send file type
 *     - should send filename
 * 12. Clean up writeTarget start
 * 13. Refactor for constants
 * 14. Multithread transmission to speed up
 */

using namespace std;
using namespace chrono;

string response, writeTarget;
uint8_t headerByte, err;

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));
TimeoutHandler timeout(1000000);

frame nodeFrame = {
    .mode = CTRL_MODE,
    .header = 0,
    .data = {0}};

vector<uint8_t> packet;
uint8_t confirmation = 0;

static volatile uint32_t nodeID;

headerData fileHeaderData;

int runFSM(bool rovMode, const char *txFile)
{
    NodeFSM node(rovMode, txFile);

    while (true)
    {
        node.update(); // Runs FSM indefinitely
    }
    return 0;
}

void SendState::handle(NodeFSM &fsm)
{
    transmit_data(audioTx, _mode, _transmit_code);

    timeout.reset();
    timeout.setDuration(_timeout_us);
    err = listen(response, &timeout);

    if (fsm.getCount() >= _maxTries - 1)
    {
        fsm.changeState(move(_failStateFactory()));
        return;
    }

    if (!err)
        err = getHeaderByte(response, headerByte);

    if (!err && headerByte == _expected_receive)
    {
        fsm.changeState(move(_nextStateFactory()));
        return;
    }

    fsm.incrCount();
}

void ReadState::handle(NodeFSM &fsm)
{
    timeout.reset();
    timeout.setDuration(_timeout_us);
    err = listen(response, &timeout);

    if (err == TIMEOUT_ERROR)
    {
        fsm.changeState(_failStateFactory());
    }
    else
    {
        err = getHeaderByte(response, headerByte);
        if (!err && headerByte == _expected_receive)
        {
            transmit_data(audioTx, CTRL_MODE, _transmit_code);
            fsm.changeState(move(_nextStateFactory()));
        }
        else
        {
            if (_send_nak)
                transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            fsm.changeState(_failStateFactory());
        }
    }
}

// Implement state transitions
void InitState::handle(NodeFSM &fsm)
{
    cout << "INIT" << endl;

    init_receiver();
    audioTx.init_stream();
    err = init_gpio();

    if (!err)
        err = init_pins("toggle");
    if (err)
    {
        cout << "GPIO ERROR" << endl;
        exit(0);
    }

    fsm.changeState(std::make_unique<IdleState>());
}

void DoneState::handle(NodeFSM &fsm)
{
    cout << "DONE" << endl;

    timeout.reset();
    timeout.setDuration(30000000);
    err = listen(response, &timeout);
    if (!err)
        err = getHeaderByte(response, headerByte);
    if (err == TIMEOUT_ERROR)
    {
        close_receiver();
        audioTx.close_stream();
        close_gpio();
        cout << "EXIT" << endl;
        exit(0);
    }
    else
    {
        if ((headerByte & 0x7F) != EOT)
            fsm.changeState(std::make_unique<IdleState>());
    }
}

// Implement state transitions
void IdleState::handle(NodeFSM &fsm)
{
    cout << "IDLE" << endl;
    if (fsm.getIsROVMode())
    {
        fsm.changeState(make_unique<SearchState>());
    }
    else
    {
        nodeID = NODE_ID;
        fsm.changeState(createSendIDState());
    }
}

void SearchState::handle(NodeFSM &fsm)
{
    cout << "SEARCH" << endl;

    timeout.reset();
    timeout.setDuration(30000000);
    err = listen(response, &timeout);

    if (err == TIMEOUT_ERROR)
        fsm.changeState(make_unique<DoneState>());
    else
        fsm.changeState(createReadIDState());
}

unique_ptr<NodeState> createSendIDState()
{
    cout << "SEND ID" << endl;
    return make_unique<SendState>(
        NODE_ID, (NODE_ID | (1 << 7)), DATA_MODE,
        []()
        { return make_unique<SendHeaderState>(); },
        []()
        { return createSendEOTState(); });
}

void SendHeaderState::handle(NodeFSM &fsm)
{
    cout << "SEND HEADER" << endl;
    timeout.reset();
    timeout.setDuration(1000000);

    const char *fileName = fsm.getFileName();
    uint32_t fileSize = (fileName[0] != '\0') ? getFileSize(fsm.getFileName()) : 0;
    headerData header = {NODE_ID, fileSize};
    err = packetFromHeaderData(packet, header);
    if (!err)
    {
        transmit_data(audioTx, DATA_MODE, HEADER_DATA,
                      packet.data(), packet.size());

        err = listen(response, &timeout);

        if (!err)
            err = getHeaderByte(response, headerByte);

        if (fsm.getCount() >= 10) // simplify this control logic somehow
        {
            fsm.changeState(createSendIDState());
        }
        else if (err)
        {
            if (err == TIMEOUT_ERROR)
                fsm.incrCount();
        }
        else if (headerByte == (HEADER_DATA | 0x80))
        {
            fsm.changeState(createSendDataStartState());
        }
    }
}

unique_ptr<NodeState> createSendDataStartState()
{
    cout << "SEND DATA START" << endl;
    return make_unique<SendState>(
        DATA_START, DATA_START, CTRL_MODE,
        []()
        { return make_unique<SendDataFrameState>(); },
        []()
        { return make_unique<SendHeaderState>(); });
}

void SendDataFrameState::handle(NodeFSM &fsm)
{
    timeout.reset();
    timeout.setDuration(100000);
    cout << "SEND DATA FRAME" << endl;
    const char *fileName = fsm.getFileName();
    if (fileName[0] != '\0')
        err = transmit_file(audioTx, fsm.getFileName(), timeout, 50);

    if (err)
        cout << "TIMED OUT" << endl;

    fsm.changeState(createSendDataDoneState());
}

unique_ptr<NodeState> createSendDataDoneState()
{
    cout << "SEND DATA DONE" << endl;
    return make_unique<SendState>(
        DATA_DONE, DATA_DONE, CTRL_MODE,
        // std::make_unique<EchoConfirmationState>(), TODO: change this back
        []()
        { return make_unique<TransmitDoneState>(); },
        []()
        { return make_unique<TransmitDoneState>(); }, 1000000, 1);
    // don't know why but doesn't receive DATA_DONE well,
    // so works better to just move on
}

unique_ptr<NodeState> createSendEOTState()
{
    cout << "SEND EOT" << endl;
    return make_unique<SendState>(
        EOT, EOT, CTRL_MODE,
        []()
        { return make_unique<DoneState>(); },
        []()
        { return make_unique<DoneState>(); },
        1000000, 5);
}

unique_ptr<NodeState> createReadIDState()
{
    cout << "READ ID" << endl;
    nodeID = NODE_ID;
    return make_unique<ReadState>(
        (NODE_ID | 0x80), (NODE_ID | 0x80),
        []()
        { return make_unique<ReadHeaderState>(); },
        []()
        { return make_unique<SearchState>(); },
        true, 10000000);
}

// TODO: clean this up
void ReadHeaderState::handle(NodeFSM &fsm)
{
    cout << "READ HEADER" << endl;
    timeout.reset();
    timeout.setDuration(5000000);

    err = listen(response, &timeout);
    if (err == TIMEOUT_ERROR)
    {
        fsm.changeState(createReadIDState());
    }
    else
    {
        err = getHeaderByte(response, headerByte);
        if (err)
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            fsm.changeState(createReadIDState());
        }
        else if ((headerByte & 0x7F) != HEADER_DATA)
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            fsm.changeState(createReadIDState());
        }
        else
        {
            string headerDataBytes;
            err = get_packet_data(response, headerDataBytes);
            if (!err && (headerByte & 0x7F) == HEADER_DATA && headerDataBytes.size() < 9)
            {
                // little endian
                uint16_t nodeId = 0 | (uint8_t)headerDataBytes.at(1) | ((uint8_t)headerDataBytes.at(2) << 8);
                uint32_t fileSize = 0;
                for (size_t i = 0; i < 4; i++)
                {
                    fileSize |= (uint8_t)headerDataBytes.at(3 + i) << (4 * i);
                }

                fileHeaderData = {
                    .nodeId = nodeId,
                    .fileSizeBytes = fileSize};
                cout << "Node ID: " << (unsigned int)fileHeaderData.nodeId << endl;
                cout << "File Size: " << (unsigned int)fileHeaderData.fileSizeBytes << endl;

                transmit_data(audioTx, DATA_MODE, HEADER_DATA);
                fsm.changeState(createReadDataStartState());
            }
            else
            {
                fsm.changeState(createReadIDState());
                transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            }
        }
    }
}

unique_ptr<NodeState> createReadDataStartState()
{
    cout << "READ DATA START" << endl;
    return make_unique<ReadState>(
        DATA_START, DATA_START,
        []()
        { return make_unique<ReadDataFrameState>(); },
        []()
        { return make_unique<ReadHeaderState>(); });
}

// TODO: move this somewhere else
string getCurrentTimeString()
{
    time_t now = std::time(nullptr);
    tm *now_tm = std::localtime(&now);

    ostringstream oss;
    oss << put_time(now_tm, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}

void ReadDataFrameState::handle(NodeFSM &fsm)
{
    timeout.reset();
    err = timeout.setDuration(100000);

    if (fileHeaderData.fileSizeBytes > 0)
    {
        writeTarget = "./received_files/node" + to_string(nodeID) + "_" + getCurrentTimeString() + ".txt";
        err = receiveFile(audioTx, writeTarget.c_str(), timeout, 20);
    }
    else
    {
        err = listen(response, &timeout); // this is scuffed a little
    }

    transmit_data(audioTx, CTRL_MODE, DATA_DONE);
    if (err == TIMEOUT_ERROR)
        fsm.changeState(createSendEOTState());
    else
        fsm.changeState(createReadEOTState());
}

unique_ptr<NodeState> createReadEOTState()
{
    cout << "READ EOT" << endl;
    return make_unique<ReadState>(
        EOT, EOT,
        []()
        { return make_unique<DoneState>(); },
        []()
        { return make_unique<DoneState>(); });
}

void TransmitDoneState::handle(NodeFSM &fsm)
{
    cout << "TRANSMIT DONE" << endl;
    timeout.reset();
    timeout.setDuration(2500000);
    err = listen(response, &timeout);

    if (!err)
        err = getHeaderByte(response, headerByte);

    if (!err)
    {
        if (headerByte == HAS_FILES)
        {
            transmit_data(audioTx, CTRL_MODE, HAS_FILES);
            fsm.changeState(createReadIDState());
            return;
        }
        else if (headerByte == HAS_NO_FILES)
        {
            transmit_data(audioTx, CTRL_MODE, HAS_NO_FILES);
            fsm.changeState(createSendEOTState());
            return;
        }
        else
        {
            err = ARGUMENT_ERROR;
        }
    }

    if (err)
    {
        transmit_data(audioTx, CTRL_MODE, NAK_SEND);
        fsm.incrCount();
    }

    if (fsm.getCount() >= 5)
    {
        fsm.changeState(createSendEOTState());
    }
}

void ReceiveDoneState::handle(NodeFSM &fsm)
{
    cout << "RECEIVE DONE" << endl;

    uint8_t hasFiles = (fsm.getFileName()[0] == '\0') ? HAS_NO_FILES : HAS_FILES;
    transmit_data(audioTx, CTRL_MODE, hasFiles);

    timeout.reset();
    timeout.setDuration(1000000);
    err = listen(response, &timeout);

    if (!err)
        err = getHeaderByte(response, headerByte);

    if (!err && headerByte == hasFiles)
    {
        fsm.changeState(
            (hasFiles == HAS_FILES)
                ? createSendIDState()
                : createReadEOTState());
        return;
    }

    fsm.incrCount();
    if (fsm.getCount() >= 5)
    {
        fsm.changeState(createReadEOTState());
    }
}