#ifndef __FILE_TRANSFER__
#define __FILE_TRANSFER__

#include "../config.h"
#include <stdint.h>
#include "../audio/audiotransmitter.h"
#include "../audio/audiorx/audioreceiver.h"
#include "../gpio/gpio.h"
#include "../timeout/timeout.h"

#if DEPLOYED
#include "../audio/audiorx/audioreceiver.h"
#else
#include "../../../tst/testlib/audioreceiver_test.h"
#endif

uint8_t transmit_file(AudioTransmitter &tx, const char *file, TimeoutHandler &timeout);

uint8_t receiveFile(AudioTransmitter &tx, const char *fileName, TimeoutHandler &timeout, uint16_t maxTries);

#if PARAMETER_TESTING

typedef struct
{
    int sent;
    int ack;
    int nak;
    int timeouts;
} TxTestData;

typedef struct
{
    int received;
    int timeouts;
    int crc_failed;
} RxTestData;

uint8_t transmit_file_test(AudioTransmitter &tx, const char *file,
                           TimeoutHandler &timeout, TxTestData *testData);

uint8_t receiveFile_test(AudioTransmitter &tx, const char *fileName,
                         TimeoutHandler &timeout, uint16_t maxTries,
                         RxTestData *testData);
#endif

#endif // __FILE_TRANSFER__