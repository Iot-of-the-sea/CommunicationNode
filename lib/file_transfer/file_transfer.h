#ifndef __FILE_TRANSFER__
#define __FILE_TRANSFER__

#include <stdint.h>
#include "../audio/audiotransmitter.h"
#include "../audio/audiorx/audioreceiver.h"

#define DEPLOYED true

#if DEPLOYED
#include "../audio/audiorx/audioreceiver.h"
#else
#include "../../../tst/testlib/audioreceiver_test.h"
#endif

uint8_t transmit_file(AudioTransmitter &tx, const char *file);

uint8_t receiveFile(AudioTransmitter &tx, const char *fileName, TimeoutHandler &timeout, uint16_t maxTries);

#endif // __FILE_TRANSFER__