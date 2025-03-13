import librosa
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import butter, filtfilt

# # Load an audio file
signal, sr = librosa.load('./output.wav', sr=None)

# Define filter function
def butter_lowpass_filter(data, cutoff, fs, order=10):
    nyquist = 0.5 * fs  # Nyquist frequency
    normal_cutoff = cutoff / nyquist  # Normalize cutoff frequency
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    filtered_data = filtfilt(b, a, data)  # Apply filter
    return filtered_data

def close_to(val, target, threshold=0.1):
    return val < target*(1+threshold) and val > target*(1-threshold)

filtered = butter_lowpass_filter(signal, 80000, sr)

spec, fqs, ts, img = plt.specgram(signal, NFFT=100, Fs=sr, noverlap=90, cmap='inferno')

print(spec.shape)
print(fqs)
print(ts)

f_sig = [fqs[np.argmax(spec[:, x])] for x in range(spec.shape[1])]

ms = 0.001
seg = []
bins = []
for i in range(spec.shape[1]):
    if ts[i] >= ms:
        bins.append(np.mean(seg))
        seg = []
        ms += 0.001
    seg.append(f_sig[i])
bins.append(np.mean(seg, dtype='uint32'))

code = []
for b in bins:
    if close_to(b, 20000):
        code.append(0)
    elif close_to(b, 40000):
        code.append(1)
    else:
        raise Exception()
code = [str(i) for i in code]
print("".join(code[::-1]))

# Plot spectrogram
plt.figure(figsize=(8, 4))
plt.plot(f_sig)
plt.show()
