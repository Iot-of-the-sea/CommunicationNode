#include "fsm.h"

using namespace std;
using namespace chrono;

string response;
uint8_t headerByte, err;

AudioTransmitter audioTx(AudioProfile(1000.0, {63000, 67000}, 50000));
TimeoutHandler timeout(1000000);

frame nodeFrame = {
    .mode = CTRL_MODE,
    .header = 0,
    .data = {0}};

vector<uint8_t> packet;
uint8_t confirmation = 0;

int runFSM(bool rovMode)
{
    NodeFSM node(rovMode);

    while (true)
    {
        node.update(); // Runs FSM indefinitely
    }
    return 0;
}

void SendState::handle(NodeFSM &fsm)
{
    transmit_data(audioTx, _mode, _transmit_code);

    timeout.setDuration(_timeout_us);
    err = listen(response, &timeout);

    if (fsm.getCount() >= _maxTries)
    {
        cout << "to fail state" << endl;
        fsm.changeState(move(_failState));
        return;
    }

    if (!err)
        err = getHeaderByte(response, headerByte);

    if (!err && headerByte == _expected_receive)
    {
        cout << "to next state" << endl;
        fsm.changeState(move(_nextState));
        return;
    }

    fsm.incrCount();
}

void ReadState::handle(NodeFSM &fsm)
{
    timeout.setDuration(_timeout_us);
    err = listen(response, &timeout);

    if (err == TIMEOUT_ERROR)
    {
        cout << "to fail state" << endl;
        fsm.changeState(move(_failState));
    }
    else
    {
        err = getHeaderByte(response, headerByte);
        if (!err && headerByte == _expected_receive)
        {
            transmit_data(audioTx, CTRL_MODE, _transmit_code);
            cout << "to next state" << endl;
            fsm.changeState(move(_nextState));
        }
        else
        {
            if (_send_nak)
                transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            cout << "to fail state" << endl;
        }
    }
}

// Implement state transitions
void IdleState::handle(NodeFSM &fsm)
{
    if (fsm.getIsROVMode())
        cout << "ready to receive? (y/n) ";
    else
        cout << "sense? (y/n) ";

    cin >> response;
    if (response == "y")
    {
        init_receiver();
        audioTx.init_stream();
        if (fsm.getIsROVMode())
        {
            cout << "to stage search" << endl;
            fsm.changeState(std::make_unique<SearchState>());
        }
        else
        {
            cout << "to stage calibrate" << endl;
            fsm.changeState(std::make_unique<CalibrateState>());
        }
    }
    else
    {
        cout << "stay in idle" << endl;
    }
}

void SearchState::handle(NodeFSM &fsm)
{
    cout << "response? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "to stage calibrate" << endl;
        fsm.changeState(std::make_unique<CalibrateState>());
    }
    else
    {
        cout << "stay in search" << endl;
    }
}

void CalibrateState::handle(NodeFSM &fsm)
{
    if (fsm.getIsROVMode())
    {
        cout << "calibrated? (y/n) ";
        cin >> response;

        if (response == "y")
        {
            transmit_data(audioTx, CTRL_MODE, ACK);

            cout << "to stage read id" << endl;
            fsm.changeState(createReadIDState());
        }
        else
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);

            cout << "stay in calibrate" << endl;
        }
    }
    else
    {
        err = listen(response, &timeout);
        if (isAck(response))
        {
            cout << "to stage send id" << endl;
            fsm.changeState(createSendIDState());
        }
        else
        {
            cout << "stay in calibrate" << endl;
        }
    }
}

unique_ptr<NodeState> createSendIDState()
{
    return make_unique<SendState>(
        NODE_ID, ACK, DATA_MODE,
        createSendRTSState(),
        std::make_unique<CalibrateState>());
}

unique_ptr<NodeState> createSendRTSState()
{
    return make_unique<SendState>(
        RTS, CTS, CTRL_MODE,
        std::make_unique<SendHeaderState>(),
        std::make_unique<IdleState>());
}

void SendHeaderState::handle(NodeFSM &fsm)
{
    uint32_t fileSize = getFileSize("./lib/test.txt");
    headerData header = {NODE_ID, fileSize};
    err = packetFromHeaderData(packet, header);
    if (!err)
    {
        transmit_data(audioTx, DATA_MODE, HEADER_DATA,
                      packet.data(), packet.size());

        timeout.setDuration(1000000);

        err = listen(response, &timeout);

        cout << (unsigned int)fsm.getCount() << endl;
        if (fsm.getCount() >= 10) // simplify this control logic somehow
        {
            cout << "return to send rts state" << endl;
            fsm.changeState(createSendRTSState());
        }
        else if (err)
        {
            if (err == TIMEOUT_ERROR)
                fsm.incrCount();
            cout << "stay in send header stage" << endl;
        }
        else if (!err && isAck(response))
        {
            cout << "to stage send data start" << endl;
            fsm.changeState(createSendDataStartState());
        }
    }
    else
    {
        cout << "stay in send header" << endl;
    }
}

unique_ptr<NodeState> createSendDataStartState()
{
    return make_unique<SendState>(
        DATA_START, ACK, CTRL_MODE,
        std::make_unique<SendDataFrameState>(),
        std::make_unique<SendHeaderState>());
}

void SendDataFrameState::handle(NodeFSM &fsm)
{
    cout << "State: SEND DATA FRAME" << endl;
    // transmit_file(audioTx, "./lib/test.txt", timeout);

    cout << "to stage echo confirmation" << endl;
    fsm.changeState(createSendDataDoneState());
}

unique_ptr<NodeState> createSendDataDoneState()
{
    return make_unique<SendState>(
        DATA_DONE, ACK, CTRL_MODE,
        std::make_unique<EchoConfirmationState>(),
        std::make_unique<IdleState>());
}

void SendEOTState::handle(NodeFSM &fsm)
{
    transmit_data(audioTx, CTRL_MODE, EOT);

    listen(response);
    if (isAck(response))
    {
        audioTx.close_stream();
        close_receiver();
        cout << "to stage idle" << endl;
        fsm.changeState(std::make_unique<IdleState>());
    }
    else
    {
        cout << "stay in send eot" << endl;
    }
}

void EchoConfirmationState::handle(NodeFSM &fsm)
{
    cout << "waiting for confirmation? (y/n) ";
    cin >> response;
    if (response == "n")
    {
        listen(response);
        if (isAck(response))
        {
            if (fsm.getIsROVMode())
            {
                cout << "to stage read eot" << endl;
                fsm.changeState(std::make_unique<ReadEOTState>());
            }
            else
            {
                cout << "even? (y/n) ";
                cin >> response;

                if (response == "y")
                {
                    cout << "to stage read rts" << endl;
                    fsm.changeState(createReadRTSState());
                }
                else
                {
                    cout << "to stage send eot" << endl;
                    fsm.changeState(std::make_unique<SendEOTState>());
                }
            }
        }
        else
        {
            cout << "stay in echo confirmation" << endl;
        }
    }
    else
    {
        cout << "stay in echo confirmation" << endl;
    }
}

unique_ptr<NodeState> createReadIDState()
{
    return make_unique<ReadState>(
        (NODE_ID | 0x80), ACK,
        createReadRTSState(),
        make_unique<CalibrateState>());
}

unique_ptr<NodeState> createReadRTSState()
{
    return make_unique<ReadState>(
        RTS, CTS,
        make_unique<ReadHeaderState>(),
        make_unique<IdleState>());
}

void ReadHeaderState::handle(NodeFSM &fsm)
{
    timeout.setDuration(5000000);

    err = listen(response, &timeout);
    if (err == TIMEOUT_ERROR)
    {
        cout << "revert to read rts stage" << endl;
        fsm.changeState(createReadRTSState());
    }
    else
    {
        err = getHeaderByte(response, headerByte);
        if (err)
        {
            cout << "error" << endl;
        }
        else if ((headerByte & 0x7F) != HEADER_DATA)
        {
            cout << "bad header byte" << endl;
        }
        else
        {
            string headerDataBytes;
            err = get_packet_data(response, headerDataBytes);
            if (!err && (headerByte & 0x7F) == HEADER_DATA && headerDataBytes.size() < 9)
            {
                uint16_t nodeId = (uint8_t)response.at(1) | ((uint8_t)response.at(2) << 4);
                cout << "Node ID: " << (unsigned char)nodeId << endl;
                uint32_t fileSize = 0;
                for (size_t i = 0; i < 4; i++)
                {
                    fileSize |= (uint8_t)response.at(3 + i) << (4 * i);
                }
                cout << "File Size: " << (unsigned char)fileSize << endl;

                transmit_data(audioTx, CTRL_MODE, ACK);
                cout << "to stage read data start" << endl;
                fsm.changeState(std::make_unique<ReadDataStartState>());
            }
            else
            {
                cout << "stay in read header" << endl;
                transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            }
        }
    }
}

void ReadDataStartState::handle(NodeFSM &fsm)
{
    timeout.setDuration(10000000);

    // listen(response, string(1, static_cast<char>(DATA_START)));
    cout << "in read data start" << endl;
    err = listen(response, &timeout);
    if (err == TIMEOUT_ERROR)
    {
        cout << "revert to idle stage" << endl;
        fsm.changeState(std::make_unique<ReadHeaderState>());
    }
    else
    {
        err = getHeaderByte(response, headerByte);
        if (!err && headerByte == DATA_START)
        {
            transmit_data(audioTx, CTRL_MODE, ACK);
            cout << "to stage read data frames" << endl;
            fsm.changeState(std::make_unique<ReadDataFrameState>());
        }
        else
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            cout << "stay in read data start" << endl;
        }
    }
}

void ReadDataFrameState::handle(NodeFSM &fsm)
{
    // listen(response, string(1, static_cast<char>(DATA_DONE)));
    listen(response);
    err = getHeaderByte(response, headerByte);
    if (!err && headerByte == DATA_DONE)
    {
        bool hasFile = true; // TODO: change to variable
        confirmation = generate_parity_byte(hasFile);
        uint8_t confirmation_arr[1] = {confirmation};
        transmit_data(audioTx, DATA_MODE, 0x00, confirmation_arr, 1);
        cout << "to stage read confirmation" << endl;
        fsm.changeState(std::make_unique<ReadConfirmationState>());
    }
    else
    {
        if (check_received_crc(response))
        {
            string packet_data;
            transmit_data(audioTx, CTRL_MODE, ACK);
            get_packet_data(response, packet_data);
            cout << packet_data << endl;
        }

        cout << "stay in read data frames" << endl;
    }
}

void ReadConfirmationState::handle(NodeFSM &fsm)
{
    cout << "max tries? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "confirmation code" << endl;
        cout << "stay in read confirmation" << endl;
    }
    else
    {
        cout << "match? (y/n) ";
        cin >> response;

        if (response == "y")
        {
            transmit_data(audioTx, CTRL_MODE, ACK);
            if (fsm.getIsROVMode())
            {
                cout << "transmit? (y/n) ";
                cin >> response;

                if (response == "y")
                {
                    cout << "to stage send rts" << endl;
                    fsm.changeState(createSendRTSState());
                }
                else
                {
                    cout << "to stage read eot" << endl;
                    fsm.changeState(std::make_unique<ReadEOTState>());
                }
            }
            else
            {
                cout << "to stage send EOT" << endl;
                fsm.changeState(std::make_unique<SendEOTState>());
            }
        }
        else
        {
            transmit_data(audioTx, CTRL_MODE, NAK_SEND);
            cout << "stay in read confirmation" << endl;
        }
    }
}

void ReadEOTState::handle(NodeFSM &fsm)
{
    // listen(response, string(1, static_cast<char>(EOT)));
    listen(response);
    err = getHeaderByte(response, headerByte);
    if (!err && headerByte == EOT)
    {
        transmit_data(audioTx, CTRL_MODE, ACK);
        audioTx.close_stream();
        close_receiver();
        cout << "to stage idle" << endl;
        fsm.changeState(std::make_unique<IdleState>());
    }
    else
    {
        transmit_data(audioTx, CTRL_MODE, NAK_SEND);
        cout << "stay in read eot" << endl;
    }
}