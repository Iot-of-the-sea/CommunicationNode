#include "audioreceiver_test.h"

string received_str;
std::mt19937 generator(std::random_device{}());
std::uniform_int_distribution<int> distribution(0, 1);

uint8_t listen(string &result)
{
    result = (char)(distribution(generator) ? ACK : NAK_SEND);
    return 0;
}

uint8_t listen(string &result, const string &mock_data)
{
    result = mock_data;
    return 0;
}