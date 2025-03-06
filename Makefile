
CC = g++
CFLAGS = -g -Wall -std=c++17
OBJ = main.o helper.o

all: program

program: $(OBJ)
	$(CC) $(CFLAGS) -o program $(OBJ)

audioprofile.o: ./lib/audio/audioprofile.h ./lib/audio/audioprofile.cpp
	$(CC) $(CFLAGS) -o ./lib/audio/audioprofile.o ./lib/audio/audioprofile.cpp

audiotransmitter: ./lib/audio/audiotransmitter.cpp ./lib/audio/audioprofile.o
	$(CC) $(CFLAGS) -o ./lib/audio/audiotransmitter ./lib/audio/audiotransmitter.cpp ./lib/audio/audioprofile.o -I/opt/homebrew/include -L/opt/homebrew/lib -lportaudio

clean:
	rm -f ./lib/*.o ./lib/audio/audiotransmitter