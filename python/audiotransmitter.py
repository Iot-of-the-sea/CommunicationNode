import numpy as np
import sounddevice as sd
from audioobject import AudioObject
from audioprofile import AudioProfile


class AudioTransmitter(AudioObject):

    def play_sequence(self, sequence):
        signal = self.generate_sequence(sequence)[0]
        sd.play(signal, self.audio.sample_rate)
        sd.wait()

    # assumes low channel is even bits
    def generate_sequence(self, sequence):
        bits = np.array(list(sequence)).astype(np.uint8)
        if (not all(np.logical_or(bits == 1, bits == 0))):
            return None
        
        au = self.audio

        bit_num = len(bits)
        seq_dur = bit_num * au.bit_time # sequence duration
        
        t = np.linspace(0, seq_dur, int(seq_dur * au.sample_rate), False)
        y = np.array([])
        
        # optimize this bit
        for n in range(bit_num):
            bit_start = n*au.bit_time
            if bits[n]:
                bit_wave = self.generate_high(bit_start, 0)[0]
            else:
                bit_wave = self.generate_low(bit_start, 0)[0]

            y = np.append(y, bit_wave)

        return y, t

    def generate_frequency(self, freq, start=0):
        au = self.audio

        t = np.linspace(0, au.bit_time, int(au.bit_time * au.sample_rate), False) + start
        y = np.sin(2*np.pi*freq*t) * au.amplitude

        return y, t

    def generate_low(self, start, channel):
        return self.generate_frequency(self.audio.low, start)
    
    def generate_high(self, start, channel):
        return self.generate_frequency(self.audio.high, start)


sequence = "1110101000100010010101010001010111101000100010010001000100101000" * 30
sequence = sequence[::-1]

ap = AudioProfile(1000, (120, 244))
tx = AudioTransmitter(ap)

tx.play_sequence(sequence)