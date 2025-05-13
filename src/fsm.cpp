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
 * 8. Fix no file issue
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
        err = transmit_file(audioTx, fsm.getFileName(), timeout, 20);

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
        { return createSendEOTState(); },
        []()
        { return createSendEOTState(); });
}

unique_ptr<NodeState> createSendEOTState()
{
    cout << "SEND EOT" << endl;
    return make_unique<SendState>(
        EOT, EOT, CTRL_MODE,
        []()
        { return make_unique<IdleState>(); },
        []()
        { return make_unique<DoneState>(); },
        1000000, 5);
}

// void EchoConfirmationState::handle(NodeFSM &fsm)
// {
//     cout << "waiting for confirmation? (y/n) ";
//     cin >> response;
//     if (response == "n")
//     {
//         listen(response);
//         if (isAck(response))
//         {
//             if (fsm.getIsROVMode())
//             {
//                 cout << "to stage read eot" << endl;
//                 fsm.changeState(createReadEOTState());
//             }
//             else
//             {
//                 cout << "even? (y/n) ";
//                 cin >> response;

//                 if (response == "y")
//                 {
//                     cout << "to stage read rts" << endl;
//                     fsm.changeState(createReadIDState());
//                 }
//                 else
//                 {
//                     cout << "to stage send eot" << endl;
//                     fsm.changeState(createSendEOTState());
//                 }
//             }
//         }
//         else
//         {
//             cout << "stay in echo confirmation" << endl;
//         }
//     }
//     else
//     {
//         cout << "stay in echo confirmation" << endl;
//     }
// }

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
                uint16_t nodeId = (uint8_t)response.at(1) | ((uint8_t)response.at(2) << 4);
                uint32_t fileSize = 0;
                for (size_t i = 0; i < 4; i++)
                {
                    fileSize |= (uint8_t)response.at(3 + i) << (4 * i);
                }

                fileHeaderData = {nodeId, fileSize};
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

    if (fileHeaderData.fileSizeBytes > 0) {
        writeTarget = "./received_files/node" + to_string(nodeID) + "_" + getCurrentTimeString() + ".txt";
        err = receiveFile(audioTx, writeTarget.c_str(), timeout, 20);
    } else {
        err = listen(response, &timeout); // this is scuffed a little
    }

    // listen(response);
    // err = getHeaderByte(response, headerByte);
    // if (!err && headerByte == DATA_DONE)
    // {
    //     bool hasFile = true; // TODO: change to variable
    //     confirmation = generate_parity_byte(hasFile);
    //     uint8_t confirmation_arr[1] = {confirmation};
    //     transmit_data(audioTx, DATA_MODE, 0x00, confirmation_arr, 1);
    //     cout << "to stage read confirmation" << endl;
    //     fsm.changeState(std::make_unique<ReadConfirmationState>());
    // }
    // else
    // {
    //     if (check_received_crc(response))
    //     {
    //         string packet_data;
    //         transmit_data(audioTx, CTRL_MODE, ACK);
    //         get_packet_data(response, packet_data);
    //         cout << packet_data << endl;
    //     }

    //     cout << "stay in read data frames" << endl;
    // }
    // if (!err)
    // {
    //     cout << hex << uppercase
    //          << setw(2) << setfill('0')
    //          << (unsigned int)headerByte << " vs " << (unsigned int)DATA_DONE << endl;
    //     err = getHeaderByte(response, headerByte);
    //     if (!err && headerByte == DATA_DONE)
    //     {
    transmit_data(audioTx, CTRL_MODE, DATA_DONE);
    if (err == TIMEOUT_ERROR)
        fsm.changeState(createSendEOTState());
    else
        fsm.changeState(createReadEOTState());
    //     }
    //     else
    //     {
    //         transmit_data(audioTx, CTRL_MODE, NAK_SEND);
    //         cout << "to fail state" << endl;
    //     }
    // }
}

// void ReadConfirmationState::handle(NodeFSM &fsm)
// {
//     cout << "max tries? (y/n) ";
//     cin >> response;

//     if (response == "y")
//     {
//         cout << "confirmation code" << endl;
//         cout << "stay in read confirmation" << endl;
//     }
//     else
//     {
//         cout << "match? (y/n) ";
//         cin >> response;

//         if (response == "y")
//         {
//             transmit_data(audioTx, CTRL_MODE, ACK);
//             if (fsm.getIsROVMode())
//             {
//                 cout << "transmit? (y/n) ";
//                 cin >> response;

//                 if (response == "y")
//                 {
//                     cout << "to stage send rts" << endl;
//                     fsm.changeState(createSendRTSState());
//                 }
//                 else
//                 {
//                     cout << "to stage read eot" << endl;
//                     fsm.changeState(createSendEOTState());
//                 }
//             }
//             else
//             {
//                 cout << "to stage send EOT" << endl;
//                 fsm.changeState(createSendEOTState());
//             }
//         }
//         else
//         {
//             transmit_data(audioTx, CTRL_MODE, NAK_SEND);
//             cout << "stay in read confirmation" << endl;
//         }
//     }
// }

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