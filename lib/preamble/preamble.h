#ifndef __PREAMBLE__
#define __PREAMBLE__

#include "fft.h"
#include <cmath>
#include <iostream>
#include <numeric>
#include <cstdint>
#include <algorithm>
#include <chrono>

#include <vector>
#include <mutex>
#include <condition_variable>

class PreambleDetector
{
public:
    PreambleDetector();

    // Main function for the detection thread
    void run();

    // Called after every 192 samples to update the sliding window
    void updateBuffer(const std::vector<float> &newData);

private:
    std::vector<float> buffer; // Sliding window for 9 bits x 192 samples
    std::mutex mutex;
    std::condition_variable cond;
    bool dataReady = false; // Flag indicating new data has arrived

    std::vector<float> preambleTemplate; // Modulated signal template for "001100"

    void detectPreamble();

    // Generate BFSK preamble template
    std::vector<float> generateBFSKPreambleTemplate();

    // Generate BFSK signal for a single bit
    std::vector<int16_t> generateSineWave(double frequency, int samples);

    // Compute normalized cross-correlation
    bool crossCorrelation(const std::vector<float> &data, const std::vector<float> &templateData);
};

#endif // __PREAMBLE__