#ifndef FFT_H
#define FFT_H

#include <vector>
#include <complex>

class FFT {
public:
    /**
     * @brief Computes the FFT of the input signal and returns the frequency with the highest energy.
     *
     * @param data Input signal array, with elements of type float.
     * @param length Length of the array.
     * @param fs Sampling rate (in Hz).
     * @return float Frequency with the highest energy (in Hz).
     */
    static float computePeakFrequency(const float* data, int length, float fs);


    /**
     * @brief Recursive implementation of the Cooley-Tukey FFT algorithm.
     *
     * @param a Vector containing complex data. The computed FFT result will be stored directly in this vector.
     */
    static void fft(std::vector<std::complex<float>>& a);
};

#endif // FFT_H
