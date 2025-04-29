#ifndef __FILE_TRANSFER__
#define __FILE_TRANSFER__

#include <stdint.h>
#include "../audio/audiotransmitter.h"
#include "../audio/audiorx/audioreceiver.h"

uint8_t receiveFile(AudioTransmitter &tx, const char *fileName, TimeoutHandler &timeout, uint16_t maxTries);

#endif // __FILE_TRANSFER__