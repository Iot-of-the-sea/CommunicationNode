#include "audioprofile.h"

AudioProfile::AudioProfile(double bit_time_us, const std::vector<double> &freq_list,
                           double sample_rate, double amplitude)
    : bit_time(bit_time_us / 1e6), sample_rate(sample_rate), amplitude(amplitude),
      low(freq_list[0]), high(freq_list[1]), slice(1.0 / sample_rate) {}

// Getter for sample rate
double AudioProfile::get_sample_rate() { return sample_rate; }

// Setter for sample rate
void AudioProfile::set_sample_rate(double new_sample_rate)
{
    if (new_sample_rate <= 0)
    {
        throw std::invalid_argument("Sample rate must be positive.");
    }
    sample_rate = new_sample_rate;
    slice = 1.0 / new_sample_rate;
}

// Getter for slice
double AudioProfile::get_slice() { return slice; }

double AudioProfile::get_bit_time() { return bit_time; }

double AudioProfile::get_low() { return low; }

double AudioProfile::get_high() { return high; }

double AudioProfile::get_amplitude() { return amplitude; }

// Print function for debugging
void AudioProfile::print_info()
{
    std::cout << "Amplitude: " << amplitude << "\n"
              << "Bit Time: " << bit_time << " s\n"
              << "Sample Rate: " << sample_rate << " Hz\n"
              << "Slice: " << slice << " s\n"
              << "Low Frequencies: " << low << " Hz\n"
              << "High Frequencies: " << high << " Hz\n";
}

// // Example usage
// int test()
// {
//     try
//     {
//         std::vector<double> freq_list(2);
//         freq_list[0] = 1000;
//         freq_list[1] = 2000;
//         AudioProfile profile(500, freq_list); // 500 μs bit time

//         profile.print_info();

//         // Change sample rate
//         profile.set_sample_rate(44100);
//         std::cout << "Updated Sample Rate: " << profile.get_sample_rate() << " Hz\n";
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << '\n';
//     }

//     return 0;
// }
