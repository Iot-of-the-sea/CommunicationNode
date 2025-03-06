
CC = g++
CFLAGS = -g -Wall -std=c++17

TST_DIR = ./tst
LIB_DIR = ./lib
BUILD_DIR = ./build

all: program

audioprofile.o: $(LIB_DIR)/audio/audioprofile.h $(LIB_DIR)/audio/audioprofile.cpp
	$(CC) $(CFLAGS) -c $(LIB_DIR)/audio/audioprofile.cpp -o $(BUILD_DIR)/audioprofile.o

audiotransmitter: audioprofile.o $(LIB_DIR)/audio/audiotransmitter.cpp
	$(CC) $(CFLAGS) -c $(LIB_DIR)/audio/audiotransmitter.cpp -I/opt/homebrew/include -o $(BUILD_DIR)/audiotransmitter.o


data.o: $(LIB_DIR)/data.cpp
	$(CC) $(CFLAGS) -c $(LIB_DIR)/data.cpp -o $(BUILD_DIR)/data.o

data: data.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/data $(BUILD_DIR)/data.o


unity.o: $(TST_DIR)/unity/unity.c $(TST_DIR)/unity/unity.h
	$(CC) $(CFLAGS) -c $(TST_DIR)/unity/unity.c -o $(BUILD_DIR)/unity.o

test: unity.o $(TST_DIR)/data_tests.cpp $(LIB_DIR)/protocol.h $(LIB_DIR)/data.cpp
	$(CC) $(CFLAGS) -o $(TST_DIR)/data_tests $(BUILD_DIR)/unity.o $(TST_DIR)/data_tests.cpp $(LIB_DIR)/data.cpp

run_test: test
	$(TST_DIR)/data_tests

clean:
	rm -f **/*.o ./lib/audio/audiotransmitter