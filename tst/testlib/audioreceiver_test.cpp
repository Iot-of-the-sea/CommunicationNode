#include "audioreceiver_test.h"

string received_str;

uint8_t listen(string &result)
{
    result = (char)ACK;
    return 0;
}

uint8_t listen(string &result, const string &mock_data)
{
    result = mock_data;
    return 0;
}