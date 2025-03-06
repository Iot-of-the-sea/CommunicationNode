#include "fsm.h"

using namespace std;
using namespace chrono;

NodeFSM node(false);

AudioTransmitter audioTx(AudioProfile(5000.0, {240, 488}));

frame nodeFrame = {
    .mode = CTRL_MODE,
    .header = 0,
    .data = {}};

std::vector<uint8_t> packet;
string response;

int main()
{
    while (true)
    {
        node.update(); // Runs FSM indefinitely
    }
    return 0;
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
            cout << "ACK" << endl;
            cout << "to stage read id" << endl;
            fsm.changeState(std::make_unique<ReadIDState>());
        }
        else
        {
            cout << "NAK" << endl;
            cout << "stay in calibrate" << endl;
        }
    }
    else
    {
        cout << "ack/nak? ";
        cin >> response;

        if (response == "ack")
        {
            cout << "to stage send id" << endl;
            fsm.changeState(std::make_unique<SendIDState>());
        }
        else
        {
            cout << "stay in calibrate" << endl;
        }
    }
}

void SendIDState::handle(NodeFSM &fsm)
{
    cout << "ack/nak? ";
    cin >> response;

    if (response == "ack")
    {
        cout << "to stage send rts" << endl;
        fsm.changeState(std::make_unique<SendRTSState>());
    }
    else
    {
        cout << "stay in send id" << endl;
    }
}

void SendRTSState::handle(NodeFSM &fsm)
{
    updateFrame(nodeFrame, CTRL_MODE, RTS);
    packetFromFrame(packet, nodeFrame);
    audioTx.play_sequence(packet, true);

    cout << "cts? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "to stage send header" << endl;
        fsm.changeState(std::make_unique<SendHeaderState>());
    }
    else
    {
        cout << "stay in send rts" << endl;
    }
}

void SendHeaderState::handle(NodeFSM &fsm)
{
    cout << "ack/nak? ";
    cin >> response;

    if (response == "ack")
    {
        cout << "to stage send data start" << endl;
        fsm.changeState(std::make_unique<SendDataStartState>());
    }
    else
    {
        cout << "stay in send header" << endl;
    }
}

void SendDataStartState::handle(NodeFSM &fsm)
{
    updateFrame(nodeFrame, CTRL_MODE, DATA_START);
    packetFromFrame(packet, nodeFrame);
    audioTx.play_sequence(packet, true);

    cout << "ack/nak? ";
    cin >> response;

    if (response == "ack")
    {
        cout << "to stage send data" << endl;
        fsm.changeState(std::make_unique<SendDataFrameState>());
    }
    else
    {
        cout << "stay in send data start" << endl;
    }
}

void SendDataFrameState::handle(NodeFSM &fsm)
{
    std::ifstream ifile("./lib/test.txt", std::ifstream::binary); // Open the file
    if (!ifile)
    {
        std::cerr << "Error opening file!" << std::endl;
    }

    char *frameBuf = new char[FRAME_SIZE_BYTES];
    uint8_t frameNum = 0;
    while (ifile.read(frameBuf, FRAME_SIZE_BYTES))
    {
        updateFrame(nodeFrame, DATA_MODE, frameNum, reinterpret_cast<uint8_t *>(frameBuf));
        packetFromFrame(packet, nodeFrame);
        audioTx.play_sequence(packet, true);
        frameNum++;
        usleep(100);
    }
    updateFrame(nodeFrame, DATA_MODE, frameNum, reinterpret_cast<uint8_t *>(frameBuf));
    packetFromFrame(packet, nodeFrame);
    audioTx.play_sequence(packet, true);

    ifile.close(); // Close the file

    cout << "done? (y/n) ";
    cin >> response;
    if (response == "y")
    {
        cout << "to stage echo confirmation" << endl;
        fsm.changeState(std::make_unique<EchoConfirmationState>());
    }
    else
    {
        cout << "stay in send data frame" << endl;
    }
}

void SendEOTState::handle(NodeFSM &fsm)
{
    updateFrame(nodeFrame, CTRL_MODE, EOT);
    packetFromFrame(packet, nodeFrame);
    audioTx.play_sequence(packet, true);

    cout << "ack/nak? ";
    cin >> response;

    if (response == "ack")
    {
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
        cout << "ack/nak? ";
        cin >> response;

        if (response == "ack")
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
                    fsm.changeState(std::make_unique<ReadRTSState>());
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

void ReadIDState::handle(NodeFSM &fsm)
{
    cout << "valid? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "ACK" << endl;
        cout << "to stage read rts" << endl;
        fsm.changeState(std::make_unique<ReadRTSState>());
    }
    else
    {
        cout << "NAK" << endl;
        cout << "stay in read id" << endl;
    }
}

void ReadRTSState::handle(NodeFSM &fsm)
{
    cout << "rts? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        updateFrame(nodeFrame, CTRL_MODE, CTS);
        packetFromFrame(packet, nodeFrame);
        audioTx.play_sequence(packet, true);

        cout << "CTS" << endl;
        cout << "to stage read header" << endl;
        fsm.changeState(std::make_unique<ReadHeaderState>());
    }
    else
    {
        cout << "stay in read rts" << endl;
    }
}

void ReadHeaderState::handle(NodeFSM &fsm)
{
    cout << "valid? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "ACK" << endl;
        cout << "to stage read data start" << endl;
        fsm.changeState(std::make_unique<ReadDataStartState>());
    }
    else
    {
        cout << "NAK" << endl;
        cout << "stay in read header" << endl;
    }
}

void ReadDataStartState::handle(NodeFSM &fsm)
{
    cout << "valid? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "ACK" << endl;
        cout << "to stage read data frames" << endl;
        fsm.changeState(std::make_unique<ReadDataFrameState>());
    }
    else
    {
        cout << "NAK" << endl;
        cout << "stay in read data start" << endl;
    }
}

void ReadDataFrameState::handle(NodeFSM &fsm)
{
    cout << "done? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "confirmation code" << endl;
        cout << "to stage read confirmation" << endl;
        fsm.changeState(std::make_unique<ReadConfirmationState>());
    }
    else
    {
        cout << "good crc? (y/n) ";
        cin >> response;

        if (response == "y")
        {
            cout << "ACK";
        }
        else
        {
            cout << "NAK";
        }
        cout << endl;
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
            cout << "ACK" << endl;
            if (fsm.getIsROVMode())
            {
                cout << "transmit? (y/n) ";
                cin >> response;

                if (response == "y")
                {
                    cout << "to stage send rts" << endl;
                    fsm.changeState(std::make_unique<SendRTSState>());
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
            cout << "NAK" << endl;
            cout << "stay in read confirmation" << endl;
        }
    }
}

void ReadEOTState::handle(NodeFSM &fsm)
{
    cout << "valid? (y/n) ";
    cin >> response;

    if (response == "y")
    {
        cout << "ACK" << endl;
        cout << "to stage idle" << endl;
        fsm.changeState(std::make_unique<IdleState>());
    }
    else
    {
        cout << "NAK" << endl;
        cout << "stay in read eot" << endl;
    }
}