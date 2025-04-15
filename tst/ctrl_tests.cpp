#include "./unity/unity.h"

#include <vector>
#include <stdint.h>
#include "../lib/control.h"

#include <iostream>

using namespace std;

void setUp() {}    // Runs before each test
void tearDown() {} // Runs after each test

void isAck_shouldReturnTrue_fromOneByteStrAck()
{
    string str;
    str.push_back((char)(0x7f));
    TEST_ASSERT_EQUAL(1, str.size());

    TEST_ASSERT_TRUE(isAck(str));
}

void isAck_shouldReturnFalse_fromManyOneByteStrNaks()
{
    uint8_t vals[] = {0x00, 0xc2, 0x72, 0x12, 0x11, 0x4e, 0xff, 0xf0};

    string str;
    for (char val : vals)
    {
        str = val;
        TEST_ASSERT_EQUAL_MESSAGE(1, str.size(), "string has more than 1 char");

        TEST_ASSERT_FALSE_MESSAGE(isAck(str), "char is ack");
    }
}

void isAck_shouldReturnFalse_fromEmpty()
{
    string str;
    TEST_ASSERT_EQUAL_MESSAGE(0, str.size(), "string has no char");

    TEST_ASSERT_FALSE(isAck(str));
}

void isAck_shouldReturnFalse_fromMultipleByteStrs()
{
    vector<string> vals = {"abc", "0xc2fe2901", "00", "0x122"};

    string str;
    for (string &val : vals)
    {
        str = val;
        TEST_ASSERT_NOT_EQUAL_MESSAGE(1, str.size(), "string has 1 char");
        TEST_ASSERT_FALSE(isAck(str));
    }
}

void isAck_shouldReturnTrue_fromOneByteIntAck()
{
    TEST_ASSERT_TRUE(isAck((uint8_t)(0x7f)));
}

void isAck_shouldReturnFalse_fromManyOneByteIntNaks()
{
    uint8_t vals[] = {0x00, 0xc2, 0x72, 0x12, 0x11, 0x4e, 0xff, 0xf0};

    for (uint8_t val : vals)
    {
        TEST_ASSERT_FALSE_MESSAGE(isAck(val), "char is ack");
    }
}

void getHeaderByte_shouldReturnEmptyPacketError_fromEmptyPacket()
{
    string emptyPacket;
    uint8_t header;
    uint8_t status;

    status = getHeaderByte(emptyPacket, header);

    TEST_ASSERT_EQUAL(EMPTY_PACKET_ERROR, status);
    TEST_ASSERT_EQUAL(0, header);
}

void getHeaderByte_shouldReturnSuccess_and_correctHeader_givenFullPacket()
{
    string packet = "abcdefghi";
    uint8_t header;
    uint8_t status;

    status = getHeaderByte(packet, header);

    TEST_ASSERT_EQUAL(NO_ERROR, status);
    TEST_ASSERT_EQUAL(97, header);
}

void getHeaderByte_shouldReturnSuccess_and_correctHeader_givenOneBytePacket()
{
    string packet = "0";
    uint8_t header;
    uint8_t status;

    status = getHeaderByte(packet, header);

    TEST_ASSERT_EQUAL(NO_ERROR, status);
    TEST_ASSERT_EQUAL(48, header);
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(isAck_shouldReturnTrue_fromOneByteStrAck);
    RUN_TEST(isAck_shouldReturnFalse_fromManyOneByteStrNaks);
    RUN_TEST(isAck_shouldReturnFalse_fromEmpty);
    RUN_TEST(isAck_shouldReturnFalse_fromMultipleByteStrs);
    RUN_TEST(isAck_shouldReturnTrue_fromOneByteIntAck);
    RUN_TEST(isAck_shouldReturnFalse_fromManyOneByteIntNaks);

    RUN_TEST(getHeaderByte_shouldReturnEmptyPacketError_fromEmptyPacket);
    RUN_TEST(getHeaderByte_shouldReturnSuccess_and_correctHeader_givenFullPacket);
    RUN_TEST(getHeaderByte_shouldReturnSuccess_and_correctHeader_givenOneBytePacket);

    return UNITY_END();
}