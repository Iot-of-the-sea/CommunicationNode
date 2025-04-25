#include "./unity/unity.h"
#include "../lib/data.h"

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
    TEST_ASSERT_EQUAL(1, emptySignal.size()); // TODO: change when optimize for size

    TEST_ASSERT_EQUAL(0, emptySignal.at(0));
}

void findCrc_givenString_shouldReturnCRC()
{
    std::string testString = "hello world";
    crc_t crc = find_crc(testString);
    TEST_ASSERT_EQUAL(0xa8, crc);

    testString = "abcdefghijklnmopqrstuvwxyz123456";
    crc = find_crc(testString);
    TEST_ASSERT_EQUAL(0x7f, crc);
}

void findCrc_givenLongString_shouldReturnCRC()
{
    std::string testString = "my name is miguel ohara and im this dimensions one and only spiderman";
    crc_t crc = find_crc(testString);
    TEST_ASSERT_EQUAL(0x30, crc);
}

void findCrc_givenShortString_shouldReturnCRC()
{
    std::string testString = "a";
    crc_t crc = find_crc(testString);
    TEST_ASSERT_EQUAL(0x20, crc);
}

void findCrc_givenEmptyString_shouldReturnCRC()
{
    std::string testString = "";
    crc_t crc = find_crc(testString);
    TEST_ASSERT_EQUAL(0x00, crc);
}

void findCrc_givenUint8Vec_shouldReturnCRC()
{
    std::vector<uint8_t> testArr = {0X68, 0X65, 0X6C, 0X6C, 0X6F, 0X20, 0X77, 0X6F, 0X72, 0X6C, 0X64};
    crc_t crc = find_crc(testArr);
    TEST_ASSERT_EQUAL(0xa8, crc);

    testArr = {0X61, 0X62, 0X63, 0X64, 0X65, 0X66, 0X67, 0X68, 0X69, 0X6A, 0X6B, 0X6C, 0X6E, 0X6D,
               0X6F, 0X70, 0X71, 0X72, 0X73, 0X74, 0X75, 0X76, 0X77, 0X78, 0X79, 0X7A, 0X31, 0X32, 0X33, 0X34, 0X35, 0X36};
    crc = find_crc(testArr);
    TEST_ASSERT_EQUAL(0x7f, crc);
}

void findCrc_givenLongUint8Vec_shouldReturnCRC()
{
    std::vector<uint8_t> testArr = {0X6D, 0X79, 0X20, 0X6E, 0X61, 0X6D, 0X65, 0X20, 0X69, 0X73, 0X20, 0X6D, 0X69, 0X67, 0X75, 0X65, 0X6C,
                                    0X20, 0X6F, 0X68, 0X61, 0X72, 0X61, 0X20, 0X61, 0X6E, 0X64, 0X20, 0X69, 0X6D, 0X20, 0X74, 0X68, 0X69, 0X73,
                                    0X20, 0X64, 0X69, 0X6D, 0X65, 0X6E, 0X73, 0X69, 0X6F, 0X6E, 0X73, 0X20, 0X6F, 0X6E, 0X65, 0X20, 0X61, 0X6E,
                                    0X64, 0X20, 0X6F, 0X6E, 0X6C, 0X79, 0X20, 0X73, 0X70, 0X69, 0X64, 0X65, 0X72, 0X6D, 0X61, 0X6E};
    crc_t crc = find_crc(testArr);
    TEST_ASSERT_EQUAL(0x30, crc);
}

void findCrc_givenShortUint8Vec_shouldReturnCRC()
{
    std::vector<uint8_t> testArr = {0X61};
    crc_t crc = find_crc(testArr);
    TEST_ASSERT_EQUAL(0x20, crc);
}

void findCrc_givenEmptyUint8Vec_shouldReturnCRC()
{
    std::vector<uint8_t> testArr = {};
    crc_t crc = find_crc(testArr);
    TEST_ASSERT_EQUAL(0x00, crc);
}

void checkReceivedCRC_givenCorrectCRC_shouldReturnTrue()
{
    std::vector<uint8_t> testArr = {0X61, 0x20};
    string testString1(reinterpret_cast<const char *>(testArr.data()), testArr.size());
    TEST_ASSERT_TRUE(check_received_crc(testString1));

    testArr = {0x00};
    string testString2(reinterpret_cast<const char *>(testArr.data()), testArr.size());
    TEST_ASSERT_TRUE(check_received_crc(testString2));
}

void checkReceivedCRC_givenIncorrectCRC_shouldReturnFalse()
{
    std::vector<uint8_t> testArr = {0X60, 0x20};
    string testString1(reinterpret_cast<const char *>(testArr.data()), testArr.size());
    TEST_ASSERT_FALSE(check_received_crc(testString1));

    testArr = {0X61, 0x21};
    string testString2(reinterpret_cast<const char *>(testArr.data()), testArr.size());
    TEST_ASSERT_FALSE(check_received_crc(testString2));
}

void packetFromFrame_givenValidDataFrame_shouldMakeValidPacket()
{
    frame testFrame = {
        .mode = DATA_MODE,
        .header = 0X21,
        .data = {0X61, 0X62, 0X63, 0X64, 0X65, 0X66, 0X67, 0X68,
                 0X69, 0X6A, 0X6B, 0X6C, 0X6E, 0X6D, 0X6F, 0X70,
                 0X71, 0X72, 0X73, 0X74, 0X75, 0X76, 0X77, 0X78,
                 0X79, 0X7A, 0X31, 0X32, 0X33, 0X34, 0X35, 0X36}};

    vector<uint8_t> packet;
    packetFromFrame(packet, testFrame);
    TEST_ASSERT_EQUAL(0xc0, packet.back());
}

void packetFromFrame_givenValidCtrlFrame_shouldMakeValidPacket()
{
    frame testFrame = {
        .mode = CTRL_MODE,
        .header = RTS,
        .data = {}};

    vector<uint8_t> packet;
    packetFromFrame(packet, testFrame);
    TEST_ASSERT_EQUAL(0xac, packet.back());
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(packFrame_shouldPopulateSignal_fromEmpty);
    RUN_TEST(packFrame_shouldPopulateSignal_fromPacked);
    // RUN_TEST(packFrame_shouldPopulateSignal_withDefaultFrame);

    RUN_TEST(findCrc_givenString_shouldReturnCRC);
    RUN_TEST(findCrc_givenLongString_shouldReturnCRC);
    RUN_TEST(findCrc_givenShortString_shouldReturnCRC);
    RUN_TEST(findCrc_givenEmptyString_shouldReturnCRC);

    RUN_TEST(findCrc_givenUint8Vec_shouldReturnCRC);
    RUN_TEST(findCrc_givenLongUint8Vec_shouldReturnCRC);
    RUN_TEST(findCrc_givenShortUint8Vec_shouldReturnCRC);
    RUN_TEST(findCrc_givenEmptyUint8Vec_shouldReturnCRC);

    RUN_TEST(checkReceivedCRC_givenCorrectCRC_shouldReturnTrue);
    RUN_TEST(checkReceivedCRC_givenIncorrectCRC_shouldReturnFalse);

    RUN_TEST(packetFromFrame_givenValidDataFrame_shouldMakeValidPacket);
    RUN_TEST(packetFromFrame_givenValidCtrlFrame_shouldMakeValidPacket);

    return UNITY_END();
}

// int test()
// {
//     std::ifstream ifile("./lib/01102521.csv", std::ifstream::binary); // Open the file
//     std::ofstream ofile("./lib/out.txt", std::ifstream::binary);

//     if (!(ifile && ofile))
//     {
//         std::cerr << "Error opening file!" << std::endl;
//         return 1;
//     }

//     char *frameBuf = new char[FRAME_SIZE_BYTES];
//     frame dataFrame = {
//         .mode = DATA_MODE,
//         .header = 0,
//         .data = {}};
//     while (ifile.read(frameBuf, FRAME_SIZE_BYTES))
//     {
//         std::memcpy(dataFrame.data, reinterpret_cast<uint8_t *>(frameBuf), FRAME_SIZE_BYTES);
//         printFrame(dataFrame);
//         ofile.write(frameBuf, FRAME_SIZE_BYTES);
//         dataFrame.header++;
//     }
//     ofile.write(frameBuf, ifile.gcount());

//     ifile.close(); // Close the file
//     ofile.close(); // Close the file
//     return 0;
// }
