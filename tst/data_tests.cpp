#include "./unity/unity.h"
#include "../lib/data.h"

#define TEST_FILE_NAME "./tst/testFile.txt"

bool checkFileMatch(string fileName, string expectedStr)
{
    ifstream ifile(fileName, ifstream::binary); // Open the file
    if (!ifile)
    {
        cerr << "Error opening file!" << endl;
    }

    char *actualBuf = new char[expectedStr.length() * 2];

    ifile.read(actualBuf, expectedStr.length() * 2);
    ifile.close();
    return string(actualBuf) == expectedStr;
}

void setUp() {} // Runs before each test
void tearDown()
{
    // remove(TEST_FILE_NAME);
} // Runs after each test

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
    testFrame.data_len = 6;
    packFrame(emptySignal, testFrame);

    TEST_ASSERT_FALSE(err);

    TEST_ASSERT_GREATER_THAN(0, emptySignal.size());
    TEST_ASSERT_EQUAL(7, emptySignal.size());
    TEST_ASSERT_EQUAL(0b10001011, emptySignal.at(0));
    TEST_ASSERT_EQUAL(127, emptySignal.at(1));
    TEST_ASSERT_EQUAL(250, emptySignal.at(2));
    TEST_ASSERT_EQUAL(100, emptySignal.at(6));

    testFrame.mode = DATA_MODE;
    testFrame.header = 0b00101111;
    testFrame.data_len = 32;
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
    TEST_ASSERT_EQUAL(0, emptySignal.size()); // TODO: change when optimize for size

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
        .data_len = 32,
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

void initFileWriter_givenDefault_noError()
{
    FileWriter testWriter;
    TEST_ASSERT_EQUAL(0, testWriter.getFileName().length());
}

void initFileWriter_givenString_noError()
{
    FileWriter testWriter(string("testFile.txt"));
    TEST_ASSERT_EQUAL(12, testWriter.getFileName().length());
    TEST_ASSERT_EQUAL_CHAR_ARRAY("testFile.txt", testWriter.getFileName().c_str(), 12);
}

void initFileWriter_givenCharArr_noError()
{
    FileWriter testWriter("testFile.txt");
    TEST_ASSERT_EQUAL(12, testWriter.getFileName().length());
    TEST_ASSERT_EQUAL_CHAR_ARRAY("testFile.txt", testWriter.getFileName().c_str(), 12);
}

void initFileWriter_givenEmptyString_doesNotAssign()
{
    FileWriter testWriter(string(""));
    TEST_ASSERT_EQUAL(0, testWriter.getFileName().length());
    ;
}

void initFileWriter_givenEmptyCharArr_doesNotAssign()
{
    FileWriter testWriter("");
    TEST_ASSERT_EQUAL(0, testWriter.getFileName().length());
}

void open_and_close_noError()
{
    FileWriter testWriter("testFile.txt");
    uint8_t err = testWriter.open();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    err = testWriter.close();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
}

void open_givenNoFileName_returnsArgumentError()
{
    FileWriter testWriter;
    uint8_t err = testWriter.open();
    TEST_ASSERT_EQUAL(ARGUMENT_ERROR, err);
}

void close_givenNoFileName_returnsNoError()
{
    FileWriter testWriter;
    uint8_t err = testWriter.close();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
}

void doubleOpen_givenValidFileName_returnsNoError()
{
    FileWriter testWriter(TEST_FILE_NAME);
    uint8_t err = testWriter.open();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.open();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
}

void write_givenCharArr_and_noStream_returnsIOError()
{
    FileWriter testWriter;
    char *testData = "abc";
    uint8_t err = testWriter.write(testData);
    TEST_ASSERT_EQUAL(IO_ERROR, err);
}

void write_givenCharArr_and_openStream_returnsNoError()
{

    FileWriter testWriter(TEST_FILE_NAME);
    char *testData = "abc";
    uint8_t err = testWriter.open();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.write(testData);
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.close();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    TEST_ASSERT_TRUE(checkFileMatch(TEST_FILE_NAME, "abc"));

    TEST_ASSERT_EQUAL(0, remove(TEST_FILE_NAME));
}

void write_givenMultipleCharArr_and_openStream_returnsNoError()
{

    FileWriter testWriter(TEST_FILE_NAME);
    char *testData = "abc";
    uint8_t err = testWriter.open();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.write(testData);
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    testData = "123";
    err = testWriter.write(testData);
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.close();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    TEST_ASSERT_TRUE(checkFileMatch(TEST_FILE_NAME, "abc123"));

    TEST_ASSERT_EQUAL(0, remove(TEST_FILE_NAME));
}

void write_givenString_and_openStream_returnsNoError()
{

    FileWriter testWriter(TEST_FILE_NAME);
    string testData = "string abc";
    uint8_t err = testWriter.open();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.write(testData);
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.close();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    TEST_ASSERT_TRUE(checkFileMatch(TEST_FILE_NAME, "string abc"));

    TEST_ASSERT_EQUAL(0, remove(TEST_FILE_NAME));
}

void write_uint8Arr_and_openStream_returnsNoError()
{

    FileWriter testWriter(TEST_FILE_NAME);
    uint8_t testData[9] = {0X75, 0X69, 0X6E, 0X74, 0X38, 0X20, 0X61, 0X62, 0X63};
    uint8_t err = testWriter.open();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.write(testData, 9);
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    err = testWriter.close();
    TEST_ASSERT_EQUAL(NO_ERROR, err);

    TEST_ASSERT_TRUE(checkFileMatch(TEST_FILE_NAME, "uint8 abc"));

    TEST_ASSERT_EQUAL(0, remove(TEST_FILE_NAME));
}

void frameFromHeaderData_givenEmptyHeader_givesZeroData()
{
    frame testFrame;
    headerData testData;
    uint8_t err = frameFromHeaderData(testFrame, testData);

    TEST_ASSERT_EQUAL_UINT8(NO_ERROR, err);
    TEST_ASSERT_EQUAL_UINT8(0x00, testFrame.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, testFrame.data[1]);

    TEST_ASSERT_EQUAL_UINT8(0x00, testFrame.data[2]);
    TEST_ASSERT_EQUAL_UINT8(0x00, testFrame.data[3]);
    TEST_ASSERT_EQUAL_UINT8(0x00, testFrame.data[4]);
    TEST_ASSERT_EQUAL_UINT8(0x00, testFrame.data[5]);
}

void frameFromHeaderData_givenFullHeader_returnsArgError()
{
    frame testFrame;
    headerData testData = {0xAB12, 0x12345678};
    uint8_t err = frameFromHeaderData(testFrame, testData);

    TEST_ASSERT_EQUAL_UINT8(NO_ERROR, err);
    TEST_ASSERT_EQUAL_UINT8(0x12, testFrame.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0xab, testFrame.data[1]);

    TEST_ASSERT_EQUAL_UINT8(0x78, testFrame.data[2]);
    TEST_ASSERT_EQUAL_UINT8(0x56, testFrame.data[3]);
    TEST_ASSERT_EQUAL_UINT8(0x34, testFrame.data[4]);
    TEST_ASSERT_EQUAL_UINT8(0x12, testFrame.data[5]);
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

    RUN_TEST(initFileWriter_givenDefault_noError);
    RUN_TEST(initFileWriter_givenString_noError);
    RUN_TEST(initFileWriter_givenCharArr_noError);
    RUN_TEST(initFileWriter_givenEmptyString_doesNotAssign);
    RUN_TEST(initFileWriter_givenEmptyCharArr_doesNotAssign);
    RUN_TEST(open_and_close_noError);
    RUN_TEST(doubleOpen_givenValidFileName_returnsNoError);
    RUN_TEST(open_givenNoFileName_returnsArgumentError);
    RUN_TEST(close_givenNoFileName_returnsNoError);
    RUN_TEST(write_givenCharArr_and_noStream_returnsIOError);
    RUN_TEST(write_givenCharArr_and_openStream_returnsNoError);
    RUN_TEST(write_givenMultipleCharArr_and_openStream_returnsNoError);
    RUN_TEST(write_givenString_and_openStream_returnsNoError);
    RUN_TEST(write_uint8Arr_and_openStream_returnsNoError);

    RUN_TEST(frameFromHeaderData_givenEmptyHeader_givesZeroData);
    RUN_TEST(frameFromHeaderData_givenFullHeader_returnsArgError);

    return UNITY_END();
}