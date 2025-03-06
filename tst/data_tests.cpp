#include "./unity/unity.h"

#include <vector>
#include <stdint.h>
#include "../lib/protocol.h"

void setUp() {}    // Runs before each test
void tearDown() {} // Runs after each test

void packFrame_shouldPopulateSignal_fromEmpty()
{
    std::vector<uint8_t> emptySignal;
    TEST_ASSERT_EQUAL(0, emptySignal.size());

    frame testFrame = {
        .mode = CTRL_MODE,
        .header = ACK,
        .data = {}};

    int err = packFrame(emptySignal, testFrame);

    TEST_ASSERT_FALSE(err);

    TEST_ASSERT_GREATER_THAN(0, emptySignal.size());
    TEST_ASSERT_EQUAL(1, emptySignal.size());

    TEST_ASSERT_EQUAL(0b1111111, emptySignal.at(0));
}

void packFrame_shouldPopulateSignal_fromPacked()
{
    std::vector<uint8_t> emptySignal;
    TEST_ASSERT_EQUAL(0, emptySignal.size());

    frame testFrame = {
        .mode = CTRL_MODE,
        .header = ACK,
        .data = {}};

    int err = packFrame(emptySignal, testFrame);

    TEST_ASSERT_FALSE(err);

    TEST_ASSERT_GREATER_THAN(0, emptySignal.size());
    TEST_ASSERT_EQUAL(1, emptySignal.size());

    TEST_ASSERT_EQUAL(0b1111111, emptySignal.at(0));

    testFrame.mode = DATA_MODE;
    testFrame.header = 0b00001011;
    testFrame.data[0] = 127;
    testFrame.data[1] = 250;
    testFrame.data[5] = 100;
    packFrame(emptySignal, testFrame);

    TEST_ASSERT_FALSE(err);

    TEST_ASSERT_GREATER_THAN(0, emptySignal.size());
    TEST_ASSERT_EQUAL(33, emptySignal.size());

    TEST_ASSERT_EQUAL(0b10001011, emptySignal.at(0));
    TEST_ASSERT_EQUAL(127, emptySignal.at(1));
    TEST_ASSERT_EQUAL(250, emptySignal.at(2));
    TEST_ASSERT_EQUAL(100, emptySignal.at(6));

    testFrame.mode = DATA_MODE;
    testFrame.header = 0b00101111;
    testFrame.data[1] = 222;
    testFrame.data[2] = 10;
    testFrame.data[6] = 0;
    testFrame.data[31] = 75;
    testFrame.data[30] = 1;
    packFrame(emptySignal, testFrame);

    TEST_ASSERT_FALSE(err);

    TEST_ASSERT_GREATER_THAN(0, emptySignal.size());
    TEST_ASSERT_EQUAL(33, emptySignal.size());

    TEST_ASSERT_EQUAL(0b10101111, emptySignal.at(0));
    TEST_ASSERT_EQUAL(127, emptySignal.at(1));
    TEST_ASSERT_EQUAL(222, emptySignal.at(2));
    TEST_ASSERT_EQUAL(100, emptySignal.at(6));
    TEST_ASSERT_EQUAL(0, emptySignal.at(7));
    TEST_ASSERT_EQUAL(75, emptySignal.at(32));
    TEST_ASSERT_EQUAL(1, emptySignal.at(31));

    testFrame.mode = CTRL_MODE;
    testFrame.header = RTS;
    testFrame.data[0] = 20;

    packFrame(emptySignal, testFrame);

    TEST_ASSERT_FALSE(err);

    TEST_ASSERT_EQUAL(1, emptySignal.size());
    TEST_ASSERT_EQUAL(0b01010101, emptySignal.at(0));
}

void packFrame_shouldPopulateSignal_withDefaultFrame()
{
    std::vector<uint8_t> emptySignal;
    TEST_ASSERT_EQUAL(0, emptySignal.size());

    frame testFrame;

    int err = packFrame(emptySignal, testFrame);

    TEST_ASSERT_FALSE(err);

    TEST_ASSERT_GREATER_THAN(0, emptySignal.size());
    TEST_ASSERT_EQUAL(1, emptySignal.size());

    TEST_ASSERT_EQUAL(0, emptySignal.at(0));
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(packFrame_shouldPopulateSignal_fromEmpty);
    RUN_TEST(packFrame_shouldPopulateSignal_fromPacked);
    RUN_TEST(packFrame_shouldPopulateSignal_withDefaultFrame);

    return UNITY_END();
}