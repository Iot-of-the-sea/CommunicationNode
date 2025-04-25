#include "unity/unity.h"
#include <unistd.h>

#include "../lib/timeout/timeout.h"

using namespace std;

void setUp() {}    // Runs before each test
void tearDown() {} // Runs after each test

void initTimeout_shouldCreateDefault_fromDefaultConstructor()
{
    TimeoutHandler test;
    TEST_ASSERT_EQUAL(IDLE, test.peepState());
    TEST_ASSERT_EQUAL(0, test.getDuration());
    TEST_ASSERT_FALSE(test.getTriggered());
}

void initTimeout_shouldCreateDefault_fromArgsConstructor()
{
    TimeoutHandler test(1000);
    TEST_ASSERT_EQUAL(IDLE, test.peepState());
    TEST_ASSERT_EQUAL(1000, test.getDuration());
    TEST_ASSERT_FALSE(test.getTriggered());
}

void start_and_peep_shouldReturnActive_givenNoDuration()
{
    TimeoutHandler test;
    TEST_ASSERT_EQUAL(IDLE, test.peepState());
    TEST_ASSERT_EQUAL(0, test.getDuration());

    uint8_t err = test.start();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(ACTIVE, test.peepState());
}

void start_and_check_shouldReturnTimeout_givenNoDuration()
{
    TimeoutHandler test;
    TEST_ASSERT_EQUAL(IDLE, test.peepState());
    TEST_ASSERT_EQUAL(0, test.getDuration());

    uint8_t err = test.start();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(TIMEOUT_ERROR, test.checkTimeout());
    TEST_ASSERT_TRUE(test.getTriggered());
}

void start_and_check_shouldReturnCorrectTimeout_given1s()
{
    TimeoutHandler test(1000000);

    uint8_t err = test.start();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(NO_TIMEOUT, test.checkTimeout());
    TEST_ASSERT_FALSE(test.getTriggered());

    usleep(1000001);
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(TIMEOUT_ERROR, test.checkTimeout());
    TEST_ASSERT_TRUE(test.getTriggered());
}

void start_and_check_shouldReturnNoTimeout_and_idle_given1s_andNoStart()
{
    TimeoutHandler test(1000000);

    TEST_ASSERT_EQUAL(NO_TIMEOUT, test.checkTimeout());
    TEST_ASSERT_EQUAL(IDLE, test.peepState());
}

void setDuration_shouldSetCorrectDuration()
{
    TimeoutHandler test;

    uint8_t err = test.setDuration(1000);
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(1000, test.getDuration());

    err = test.start();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(ACTIVE, test.peepState());
    // Sleep until the target time point
    auto startTime = chrono::steady_clock::now();
    while (chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - startTime).count() < 995)
    {
    }
    TEST_ASSERT_EQUAL(NO_TIMEOUT, test.checkTimeout());
    while (chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - startTime).count() < 1005)
    {
    }
    TEST_ASSERT_EQUAL(TIMEOUT_ERROR, test.checkTimeout());

    err = test.setDuration(1500);
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    err = test.start();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(ACTIVE, test.peepState());
    startTime = chrono::steady_clock::now();
    while (chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - startTime).count() < 1495)
    {
    }
    TEST_ASSERT_EQUAL(NO_TIMEOUT, test.checkTimeout());
    while (chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - startTime).count() < 1505)
    {
    }
    TEST_ASSERT_EQUAL(TIMEOUT_ERROR, test.checkTimeout());
}

void setDuration_shouldReturnStateError_whenActiveState()
{
    TimeoutHandler test(1000000);

    uint8_t err = test.start();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(NO_TIMEOUT, test.checkTimeout());

    err = test.setDuration(0);
    TEST_ASSERT_EQUAL(STATE_ERROR, err);
}

void startTimeout_given500us_shouldBehaveCorrectly()
{
    TimeoutHandler test(100);
    TEST_ASSERT_EQUAL(100, test.getDuration());

    uint8_t err = test.startTimeout(500);
    auto startTime = chrono::steady_clock::now();
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(ACTIVE, test.peepState());
    while (chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - startTime).count() < 495)
    {
    }
    TEST_ASSERT_EQUAL(NO_TIMEOUT, test.checkTimeout());
    while (chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - startTime).count() < 505)
    {
    }
    TEST_ASSERT_EQUAL(TIMEOUT_ERROR, test.checkTimeout());
}

void startTimeout_ranTwiceInSuccession_returnsStateError()
{
    TimeoutHandler test(100);
    TEST_ASSERT_EQUAL(100, test.getDuration());

    uint8_t err = test.startTimeout(500);
    TEST_ASSERT_EQUAL(NO_ERROR, err);
    TEST_ASSERT_EQUAL(ACTIVE, test.peepState());
    err = test.startTimeout(1000);
    TEST_ASSERT_EQUAL(STATE_ERROR, err);
    TEST_ASSERT_EQUAL(ACTIVE, test.peepState());
}

void clearTrigger_resetsTriggerFlag()
{
    TimeoutHandler test(100);
    test.start();
    auto startTime = chrono::steady_clock::now();
    TEST_ASSERT_FALSE(test.getTriggered());
    while (chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - startTime).count() < 100)
    {
    }
    TEST_ASSERT_EQUAL(TIMEOUT_ERROR, test.checkTimeout());
    TEST_ASSERT_TRUE(test.getTriggered());
    test.clearTriggered();
    TEST_ASSERT_FALSE(test.getTriggered());
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(initTimeout_shouldCreateDefault_fromDefaultConstructor);
    RUN_TEST(initTimeout_shouldCreateDefault_fromArgsConstructor);
    RUN_TEST(start_and_peep_shouldReturnActive_givenNoDuration);
    RUN_TEST(start_and_check_shouldReturnTimeout_givenNoDuration);
    RUN_TEST(start_and_check_shouldReturnNoTimeout_and_idle_given1s_andNoStart);
    RUN_TEST(startTimeout_ranTwiceInSuccession_returnsStateError);

    // time based
    RUN_TEST(start_and_check_shouldReturnCorrectTimeout_given1s);
    RUN_TEST(setDuration_shouldReturnStateError_whenActiveState);
    RUN_TEST(setDuration_shouldSetCorrectDuration);
    RUN_TEST(startTimeout_given500us_shouldBehaveCorrectly);
    RUN_TEST(clearTrigger_resetsTriggerFlag);

    return UNITY_END();
}