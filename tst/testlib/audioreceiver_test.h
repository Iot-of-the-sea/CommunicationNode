#ifndef __AUDIO_RECEIVER_TEST__
#define __AUDIO_RECEIVER_TEST__

#include <string.h>
#include "../../lib/protocol.h"

using namespace std;

uint8_t init_receiver();

uint8_t listen(string &result);

uint8_t listen(string &result, const string &mock_data);

uint8_t close_receiver();

#endif // __AUDIO_RECEIVER_TEST__