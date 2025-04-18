
CC = g++
CFLAGS = -g -Wall -std=c++17

TST = tst
BUILD = build

# PA = -I/opt/homebrew/include -L/opt/homebrew/lib -lportaudio
PA = -L/usr/lib/aarch64-linux-gnu -lportaudio -lasound -lm -lpthread

SRC_DIRS := src lib lib/audio lib/audio/audiorx # tst/testlib
BUILD_DIRS := $(addprefix $(BUILD)/, $(SRC_DIRS))
SRC_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp) $(wildcard $(dir)/*.c))

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(filter %.cpp, $(SRC_FILES))) \
       $(patsubst %.c,$(BUILD)/%.o,$(filter %.c,$(SRC_FILES)))

all: program

debug: debug_program

program: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o run $(PA)

$(OBJ): | $(BUILD_DIRS)

$(BUILD)/%.o: %.cpp
	$(CC) $(CFLAGS) $(PA) -c $< -o $@

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) $(PA) -c $< -o $@

$(BUILD_DIRS):
	mkdir -p $@

debug_program: $(BUILD)/lib/audio/audiotransmitter.o $(BUILD)/data.o $(BUILD)/control.o $(SRC)/fsm.cpp $(SRC)/fsm.h
	$(CC) $(CFLAGS) -o program $(SRC)/fsm.cpp $(BUILD)/audiotransmitter.o $(BUILD)/audioprofile.o $(BUILD)/data.o $(PA)


data: data.o
	$(CC) $(CFLAGS) -o $(BUILD)/data $(BUILD)/data.o

unity.o: $(TST)/unity/unity.c $(TST)/unity/unity.h | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -c $(TST)/unity/unity.c -o $(BUILD)/unity.o

data_test: unity.o $(TST)/data_tests.cpp $(BUILD)/lib/data.o
	$(CC) $(CFLAGS) -o $(TST)/data_tests $(BUILD)/unity.o $(TST)/data_tests.cpp lib/data.cpp lib/crc8.c

ctrl_test: unity.o $(TST)/ctrl_tests.cpp
	$(CC) $(CFLAGS) -o $(TST)/ctrl_tests $(BUILD)/unity.o $(TST)/ctrl_tests.cpp lib/control.cpp

file_tx_test: $(BUILD)/lib/audio/audiotransmitter.o $(BUILD)/lib/audio/audioprofile.o $(BUILD)/lib/data.o $(TST)/file_tx_test.cpp $(BUILD)/lib/control.o $(BUILD)/lib/crc8.o $(BUILD)/tst/testlib/audioreceiver_test.o
	$(CC) $(CFLAGS) -o $(TST)/file_tx_test $^ $(PA)

run_test: data_test ctrl_test
	$(TST)/data_tests
	$(TST)/ctrl_tests

clean:
	rm -rf build/

.PHONY: all clean