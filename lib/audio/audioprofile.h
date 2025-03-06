
#ifndef __AUDIO_PROFILE__
#define __AUDIO_PROFILE__

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>

class AudioProfile
{
private:
    double amplitude;
    double bit_time;
    double sample_rate;
    double slice;
    double low;
    double high;

public:
    // Constructor
    AudioProfile(double bit_time_us, const std::vector<double> &freq_list,
                 double sample_rate = 192000, double amplitude = 1.0);

    // Getter for sample rate
    double get_sample_rate();

    // Setter for sample rate
    void set_sample_rate(double new_sample_rate);

    // Getter for slice
    double get_slice();

    double get_bit_time();
    double get_low();
    double get_high();
    double get_amplitude();

    // Print function for debugging
    void print_info();
};

#endif // __AUDIO_PROFILE__
