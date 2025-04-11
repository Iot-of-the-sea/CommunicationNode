#include "PreambleDetector.h"
#include "Demodulation.h"
#include <cmath>
#include <iostream>
#include <numeric>
#include <cstdint>
#include <algorithm>
#include <chrono>

vector<float> buffer(BUFFER_SIZE * 10, 0.0f);
mutex mutex_preamble;
condition_variable cond;
vector<float> preambleTemplate;

vector<float> generateBFSKPreambleTemplate()
{
    vector<int16_t> bfskWave;
    const vector<char> preambleBits = {'0', '0', '1', '1', '0', '0', '1', '1'};

    for (char bit : preambleBits)
    {
        // double frequency = (bit == '1') ? 67000.0 : 63000.0;
        // double frequency = (bit == '1') ? 57000.0 : 53000.0;
        double frequency = (bit == '1') ? 47000.0 : 43000.0;
        // double frequency = (bit == '1') ? 9000.0 : 7000.0;
        auto wave = generateSineWave(frequency, BUFFER_SIZE);
        bfskWave.insert(bfskWave.end(), wave.begin(), wave.end());
    }

    vector<float> templateFloat;
    for (int16_t sample : bfskWave)
    {
        templateFloat.push_back(static_cast<float>(sample) / AMPLITUDE); // Normalize to [-1, 1]
    }

    return templateFloat;
}

vector<int16_t> generateSineWave(double frequency, int samples)
{
    vector<int16_t> wave(samples);
    for (int i = 0; i < samples; ++i)
    {
        wave[i] = static_cast<int16_t>(AMPLITUDE * sin(2 * PI * frequency * i / SAMPLE_RATE));
    }
    return wave;
}

void updateBuffer(const vector<float> &newData)
{
    lock_guard<mutex> lock(mutex_preamble);

    // Sliding window: discard the oldest 192 samples and append 192 new samples
    buffer.erase(buffer.begin(), buffer.begin() + BUFFER_SIZE);
    buffer.insert(buffer.end(), newData.begin(), newData.end());

    cond.notify_one(); // Wake up the run()
}

void run(string &output)
{
    std::cout << "running preamble and demodulation" << std::endl;
    preambleTemplate = generateBFSKPreambleTemplate();

    // Used to store the first valid detection result
    bool detectionInitialized = false;
    size_t bestPosition_old = 0;
    float maxCorrelation_old = 0.0f;

    while (true)
    {
        {
            unique_lock<mutex> lock(mutex_preamble);
            cond.wait(lock);
        }

        // Each time new data arrives, compute cross-correlation
        size_t bestPosition_new;
        float maxCorrelation_new;
        bool correlationDetected = crossCorrelation(buffer, preambleTemplate, bestPosition_new, maxCorrelation_new);

        if (correlationDetected)
        {
            if (!detectionInitialized)
            {
                // First time detecting a correlation above threshold: store result but don��t confirm yet
                bestPosition_old = bestPosition_new;
                maxCorrelation_old = maxCorrelation_new;
                detectionInitialized = true;
            }
            else
            {
                // Second detection: check if it satisfies both conditions:
                // (new maxCorrelation == old maxCorrelation) AND (new bestPosition == old bestPosition - 192)
                if (maxCorrelation_new == maxCorrelation_old && bestPosition_new == bestPosition_old - 192)
                {
                    std::cout << "[PreambleDetector] Preamble detected!" << std::endl;
                    size_t postIndex = bestPosition_new + preambleTemplate.size();

                    // Extract data following the preamble
                    std::vector<float> postPreambleData;
                    if (postIndex < buffer.size())
                    {
                        postPreambleData.assign(buffer.begin() + postIndex, buffer.end());
                    }

                    // Initialize Demodulation module and start demodulation thread
                    initDemodulation(postPreambleData);
                    startDemodulation(output);
                    std::cout << "Preamble detection exit!" << std::endl;
                    return;
                }
                else
                {
                    // Second detection did not meet the condition, update stored values and wait for next input
                    bestPosition_old = bestPosition_new;
                    maxCorrelation_old = maxCorrelation_new;
                }
            }
        }
        else
        {
            // If crossCorrelation returns false, reset detection state
            detectionInitialized = false;
            bestPosition_old = 0;
            maxCorrelation_old = 0.0f;
        }

        if (isDemodulationActive())
        {
            std::cout << "Preamble detection exit!" << std::endl;
            return;
        }
    }
}

// Detect the preamble: once found, extract data after the preamble, initialize the Demodulation module, and start demodulation thread
// void detectPreamble()
// {
//     size_t bestPosition_old = 0;
//     float maxCorrelation_old = 0.0f;
//     if (crossCorrelation(buffer, preambleTemplate, bestPosition_old, maxCorrelation_old))
//     {
//         // Extract data following the preamble from buffer
//         size_t postIndex = bestPosition_old + preambleTemplate.size();
//         std::vector<float> postPreambleData;
//         if (postIndex < buffer.size())
//         {
//             postPreambleData.assign(buffer.begin() + postIndex, buffer.end());
//         }
//         std::cout << "[PreambleDetector] Preamble detected!" << std::endl;

//         // Initialize Demodulation module with post-preamble data and start demodulation thread
//         initDemodulation(postPreambleData);
//         startDemodulation();

//         // Exit the detection thread. In real applications, use a flag instead of exiting directly.
//         return;
//     }
// }

// Compute cross-correlation to detect the preamble and return the best matching position
bool crossCorrelation(const std::vector<float> &data, const std::vector<float> &templateData, size_t &bestPosition, float &maxCorrelation)
{
    const size_t maxLag = data.size() - templateData.size();
    vector<float> correlationValues(maxLag + 1, 0.0f);

    float templateEnergy = 0.0f;
    for (const auto &val : templateData)
    {
        templateEnergy += val * val;
    }

    for (size_t lag = 0; lag <= maxLag; ++lag)
    {
        float correlation = 0.0f;
        float dataEnergy = 0.0f;

        for (size_t i = 0; i < templateData.size(); ++i)
        {
            correlation += data[lag + i] * templateData[i];
            dataEnergy += data[lag + i] * data[lag + i];
        }

        if (dataEnergy > 0 && templateEnergy > 0)
        {
            correlationValues[lag] = correlation / sqrt(dataEnergy * templateEnergy);
        }
        else
        {
            correlationValues[lag] = 0.0f;
        }
    }

    bestPosition = 0;
    maxCorrelation = correlationValues[0];

    for (size_t i = 1; i <= maxLag; ++i)
    {
        if (correlationValues[i] > maxCorrelation)
        {
            maxCorrelation = correlationValues[i];
            bestPosition = i;
        }
    }

    bool detectionSuccessful = (maxCorrelation >= DETECTION_THRESHOLD);

    if (detectionSuccessful)
    {
        std::cout << "Most likely starting position: " << bestPosition << std::endl;
        std::cout << "Maximum correlation value: " << maxCorrelation << std::endl;
    }

    return detectionSuccessful;
}
