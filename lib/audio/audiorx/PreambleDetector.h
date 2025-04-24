#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
using namespace std;

constexpr int BUFFER_SIZE = 192;    // Length of each bit (in samples)
constexpr int SAMPLE_RATE = 192000; // Sampling rate: 192 kHz
constexpr double PI = 3.14159265358979323846;
constexpr int16_t AMPLITUDE = 32767;
constexpr float DETECTION_THRESHOLD = 0.85f; // Empirical threshold - correlation above this indicates detection

// void run();
void run(string &output);
void updateBuffer(const vector<float> &newData);
void detectPreamble();
vector<float> generateBFSKPreambleTemplate();
vector<int16_t> generateSineWave(double frequency, int samples);
bool crossCorrelation(const std::vector<float> &data, const std::vector<float> &templateData, size_t &bestPosition, float &maxCorrelation);
