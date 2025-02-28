import numpy as np
from typeguard import typechecked

__all__ = ["AudioProfile"]

# TODO: TEST
# TODO: change to accept variable N-FSK channels
# TODO: change to accept custom channel frequencies
@typechecked
class AudioProfile:

    # bit time in us
    def __init__(self, bit_time, freq_list, 
                 sample_rate=192000, amplitude=1):
        # TODO: add exceptions for bad inputs
        self.amplitude = amplitude
        self.bit_time = bit_time / 1e6

        # assigning frequencies to avoid overlap
        self.low, self.high = freq_list

        self._sample_rate = sample_rate
        self._slice = 1 / sample_rate

    def _find_frequencies(self, min, max):
        freq_step = (max - min) / 3
        low = np.array([0, 1]) * freq_step + min
        high = np.array([2, 3]) * freq_step + min
        return low, high

    @property
    def sample_rate(self): return self._sample_rate
    @sample_rate.setter
    def sample_rate(self, new_sample_rate):
        self._sample_rate = new_sample_rate
        self._slice = 1 / new_sample_rate

    @property
    def slice(self): return self._slice