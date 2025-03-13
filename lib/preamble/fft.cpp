#include "fft.h"

// Recursive implementation of the Cooley-Tukey FFT algorithm
void FFT::fft(std::vector<std::complex<float>>& a) {
    int n = a.size();
    if (n <= 1) return;

    // Split into even and odd parts
    std::vector<std::complex<float>> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; i++) {
        even[i] = a[i * 2];
        odd[i] = a[i * 2 + 1];
    }

    // Recursively compute FFT for even and odd parts
    fft(even);
    fft(odd);

    // Combine results
    for (int k = 0; k < n / 2; k++) {
        float angle = -2.0f * 3.14159265358979323846f * k / n;
        std::complex<float> t = std::polar(1.0f, angle) * odd[k];
        a[k] = even[k] + t;
        a[k + n / 2] = even[k] - t;
    }
}

float FFT::computePeakFrequency(const float* data, int length, float fs) {
    // Find the smallest power of 2 greater than or equal to length, to use as the FFT size
    int n = 1;
    while (n < length) {
        n *= 2;
    }

    // Copy input data into a complex vector, and pad with zeros if needed
    std::vector<std::complex<float>> x(n, 0);
    for (int i = 0; i < length; i++) {
        x[i] = std::complex<float>(data[i], 0);
    }

    // Perform FFT
    fft(x);

    // Find the frequency bin with the highest energy (magnitude squared) in the FFT result
    // Only consider the range 0 ~ fs/2 (the first n/2 + 1 points)
    int maxIndex = 0;
    float maxMagnitude = 0.0f;
    int half = n / 2;
    for (int i = 0; i <= half; i++) {
        float magnitude = std::norm(x[i]); // norm computes the squared magnitude of a complex number
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            maxIndex = i;
        }
    }

    // Compute the corresponding frequency: frequency = (index * sampling rate) / FFT size
    float frequency = (maxIndex * fs) / n;
    return frequency;
}