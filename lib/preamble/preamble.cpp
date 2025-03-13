#include "preamble.h"

constexpr int BUFFER_SIZE = 192;    // Length of each bit (in samples)
constexpr int SAMPLE_RATE = 192000; // Sampling rate: 192 kHz
constexpr double PI = 3.14159265358979323846;
constexpr int16_t AMPLITUDE = 32767;
constexpr float DETECTION_THRESHOLD = 0.8f; // Empirical threshold ¡ª correlation above this indicates detection

PreambleDetector::PreambleDetector()
{
    buffer.resize(BUFFER_SIZE * 10, 0.0f); // 10-bit window
    preambleTemplate = generateBFSKPreambleTemplate();
}

std::vector<float> PreambleDetector::generateBFSKPreambleTemplate()
{
    std::vector<int16_t> bfskWave;
    const std::vector<char> preambleBits = {'0', '0', '1', '1', '0', '0', '1', '1'};

    for (char bit : preambleBits)
    {
        double frequency = (bit == '1') ? 87000.0 : 85000.0;
        auto wave = generateSineWave(frequency, BUFFER_SIZE);
        bfskWave.insert(bfskWave.end(), wave.begin(), wave.end());
    }

    std::vector<float> templateFloat;
    for (int16_t sample : bfskWave)
    {
        templateFloat.push_back(static_cast<float>(sample) / AMPLITUDE); // Normalize to [-1, 1]
    }

    return templateFloat;
}

std::vector<int16_t> PreambleDetector::generateSineWave(double frequency, int samples)
{
    std::vector<int16_t> wave(samples);
    for (int i = 0; i < samples; ++i)
    {
        wave[i] = static_cast<int16_t>(AMPLITUDE * sin(2 * PI * frequency * i / SAMPLE_RATE));
    }
    return wave;
}

void PreambleDetector::updateBuffer(const std::vector<float> &newData)
{
    std::lock_guard<std::mutex> lock(mutex);

    // Sliding window: discard the oldest 192 samples and append 192 new samples
    buffer.erase(buffer.begin(), buffer.begin() + BUFFER_SIZE);
    buffer.insert(buffer.end(), newData.begin(), newData.end());

    dataReady = true;
    cond.notify_one(); // Wake up the detection thread
}

void PreambleDetector::run()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]
                      { return dataReady; });
            dataReady = false; // Clear flag
        }
        detectPreamble();
    }
}

void PreambleDetector::detectPreamble()
{

    if (crossCorrelation(buffer, preambleTemplate))
    {
        std::cout << "[PreambleDetector] Preamble detected!" << std::endl;
        exit(0);
    }
}

bool PreambleDetector::crossCorrelation(const std::vector<float> &data, const std::vector<float> &templateData)
{

    int dataSize = data.size();             // 10 bits
    int templateSize = templateData.size(); // 8 bits

    // === 1. Determine FFT size (next power of 2) ===
    int fftSize = 1;
    while (fftSize < dataSize)
    {
        fftSize *= 2;
    }

    // === 2. Create complex arrays and zero-pad ===
    std::vector<std::complex<float>> bufferFFT(fftSize, 0.0f);
    std::vector<std::complex<float>> templateFFT(fftSize, 0.0f);

    // Fill the signal array
    for (int i = 0; i < dataSize; ++i)
    {
        bufferFFT[i] = std::complex<float>(data[i], 0.0f);
    }
    // Fill the template array (template is shorter than buffer, so it gets zero-padded)
    for (int i = 0; i < templateSize; ++i)
    {
        templateFFT[i] = std::complex<float>(templateData[i], 0.0f);
    }

    // === 3. Compute FFT ===
    FFT::fft(bufferFFT);
    FFT::fft(templateFFT);

    // === 4. Frequency-domain conjugate multiplication ===
    for (int i = 0; i < fftSize; ++i)
    {
        bufferFFT[i] *= std::conj(templateFFT[i]);
    }

    // === 5. Inverse FFT (your FFT library uses the same function for FFT and IFFT)
    FFT::fft(bufferFFT);

    // === 6. Normalize and find peak correlation coefficient ===
    float energyTemplate = 0, energySignal = 0;
    for (float v : templateData)
    {
        energyTemplate += v * v;
    }
    for (float v : data)
    {
        energySignal += v * v;
    }

    float normFactor = std::sqrt(energyTemplate * energySignal) * fftSize;

    if (normFactor < 1e-6f)
    {
        std::cerr << "[PreambleDetector] WARNING: Zero energy detected, aborting correlation." << std::endl;
        return false;
    }

    // === Find peak correlation position ===
    float maxCorrelation = 0.0f;
    int peakIndex = -1;

    for (int i = 0; i < fftSize; ++i)
    {
        float correlation = std::abs(bufferFFT[i].real()) / normFactor;
        if (correlation > maxCorrelation)
        {
            maxCorrelation = correlation;
            peakIndex = i; // Record the peak position
        }
    }
    if (maxCorrelation > 0.3)
    {
        std::cout << "[PreambleDetector] Max FFT Correlation: " << maxCorrelation << std::endl;
    }

    if (maxCorrelation > DETECTION_THRESHOLD)
    {
        // === Calculate the actual preamble start position ===
        // This handles the "cyclic shift" issue caused by circular cross-correlation
        int preambleStart = peakIndex;
        if (preambleStart >= dataSize)
        {
            preambleStart -= fftSize; // If the peak is in the second half (wrapped around), adjust back
        }

        std::cout << "[PreambleDetector] Preamble detected at position: " << preambleStart << std::endl;
        return true;
    }

    return false;
}