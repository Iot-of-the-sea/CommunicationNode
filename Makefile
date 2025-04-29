CC = g++
DEBUG_FLAGS = -g -Wall -std=c++17
RELEASE_FLAGS = -O2 -Wall -std=c++17 -flto
CFLAGS = $(RELEASE_FLAGS)

TST = tst
BUILD = build

# PA = -I/opt/homebrew/include -L/opt/homebrew/lib -lportaudio
PA = -L/usr/lib/aarch64-linux-gnu -lportaudio -lasound -lm -lpthread

SRC_DIRS := src lib lib/audio lib/audio/audiorx # tst/testlib
TX_TST_DIRS := lib lib/audio lib/audio/audiorx tst/file_tx_tests lib/timeout lib/file_transfer lib/gpio # tst/testlib
RX_TST_DIRS := lib lib/audio lib/audio/audiorx tst/file_rx_tests lib/timeout lib/file_transfer lib/gpio # tst/testlib

TARGET_DIRS = $(RX_TST_DIRS)
BUILD_DIRS := $(addprefix $(BUILD)/, $(TARGET_DIRS))
SRC_FILES := $(foreach dir,$(TARGET_DIRS),$(wildcard $(dir)/*.cpp) $(wildcard $(dir)/*.c))

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(filter %.cpp, $(SRC_FILES))) \
       $(patsubst %.c,$(BUILD)/%.o,$(filter %.c,$(SRC_FILES)))

all: program

debug: debug_program

program: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o run $(PA) -pthread

$(OBJ): | $(BUILD_DIRS)

$(BUILD)/%.o: %.cpp
	$(CC) $(CFLAGS) $(PA) -c $< -o $@

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) $(PA) -c $< -o $@

$(BUILD_DIRS):
	mkdir -p $@

debug_program: $(BUILD)/lib/audio/audiotransmitter.o $(BUILD)/data.o $(BUILD)/control.o $(SRC)/fsm.cpp $(SRC)/fsm.h
	$(CC) $(CFLAGS) -o program $(SRC)/fsm.cpp $(BUILD)/audiotransmitter.o $(BUILD)/audioprofile.o $(BUILD)/data.o $(PA)


gpio: lib/gpio/gpio.cpp
	$(CC) $(CFLAGS) -o $(TST)/gpio_test lib/gpio/gpio.cpp -lgpiod

data: data.o
	$(CC) $(CFLAGS) -o $(BUILD)/data $(BUILD)/data.o

unity.o: $(TST)/unity/unity.c $(TST)/unity/unity.h | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -c $(TST)/unity/unity.c -o $(BUILD)/unity.o

data_test: unity.o $(TST)/data_tests.cpp $(BUILD)/lib/data.o
	$(CC) $(CFLAGS) -o $(TST)/data_tests $(BUILD)/unity.o $(TST)/data_tests.cpp lib/data.cpp lib/crc8.c

ctrl_test: unity.o $(TST)/ctrl_tests.cpp
	$(CC) $(CFLAGS) -o $(TST)/ctrl_tests $(BUILD)/unity.o $(TST)/ctrl_tests.cpp lib/control.cpp

timeout_test: unity.o $(TST)/timeout_tests.cpp
	$(CC) $(CFLAGS) -o $(TST)/timeout_tests $(BUILD)/unity.o $(TST)/timeout_tests.cpp lib/timeout/timeout.cpp

# file_tx_test: TARGET_DIRS := $(TX_TST_DIRS)
# file_rx_test: TARGET_DIRS := $(RX_TST_DIRS)

file_tx_test file_rx_test: $(OBJ)
	$(CC) $(CFLAGS) -o $(TST)/$@ $^ $(PA) -pthread -lgpiod

# file_tx_test: $(OBJ)
# 	$(CC) $(CFLAGS) -o $(TST)/$@ $^ $(PA) -pthread

# file_rx_test: $(OBJ)
# 	$(CC) $(CFLAGS) -o $(TST)/$@ $^ $(PA) -pthread

run_test: data_test ctrl_test timeout_test
	$(TST)/data_tests
	$(TST)/ctrl_tests
	$(TST)/timeout_tests

clean:
	rm -rf build/

.PHONY: all clean