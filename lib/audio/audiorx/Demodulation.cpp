#include "Demodulation.h"
#include "Sampling.h"
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>

// Internal global variables
static vector<float> demodBuffer;
static mutex demodMutex;
static condition_variable demodCond;
static thread demodThread;
static atomic<bool> demodRunning(false);

// Initialize the demodulation buffer using data after the preamble detected by PreambleDetector
// After initialization, indicate that the PreambleDetector thread has stopped (handled by PreambleDetector module)
void initDemodulation(const vector<float> &postPreambleData)
{
	lock_guard<mutex> lock(demodMutex);
	demodBuffer = postPreambleData;
#if !DEPLOYED
	cout << "Demodulation buffer initialized with " << demodBuffer.size() << " float samples." << endl;
	cout << "PreambleDetector thread closed." << endl;
#endif
}

// Add new data from the Sampling module (192 samples each time)
void addSamplingData(const vector<float> &newData)
{
	{
		lock_guard<mutex> lock(demodMutex);
		demodBuffer.insert(demodBuffer.end(), newData.begin(), newData.end());
	}
	demodCond.notify_one();
}

static uint8_t demodulationThreadFunc(string &output)
{
	string bitBuffer;
	while (demodRunning)
	{
		unique_lock<mutex> lock(demodMutex);

		// Continue only when demodBuffer.size() >= 192
		demodCond.wait(lock, []
					   { return demodBuffer.size() >= 192 || !demodRunning; });

		if (!demodRunning)
			break; // Exit the thread if a stop signal is received

		float fftSamples[192];
		copy(demodBuffer.begin(), demodBuffer.begin() + 192, fftSamples);
		demodBuffer.erase(demodBuffer.begin(), demodBuffer.begin() + 192);

		lock.unlock(); // Unlock early to prevent FFT computation from blocking other threads

		float peakFreq = FFT::computePeakFrequency(fftSamples, 192, 192000);

		// if (peakFreq >= 69750)
		if (peakFreq < 60000)
		{
			break; // Stop signal detected, exit loop
		}
		else
		{
			// Frequency in [62500, 63500] is interpreted as 0
			if (peakFreq >= 62500 && peakFreq <= 63500)
			{
				bitBuffer.push_back('0');
			}
			// Frequency in [66500, 67500] is interpreted as 1
			else if (peakFreq >= 66500 && peakFreq <= 67500)
			{
				bitBuffer.push_back('1');
			}
			// Ignore other frequencies (or add logic as needed)
			// cout << peakFreq << flush;
		}
	}

	// Convert every 8 bits in bitBuffer to one ASCII character
	string result;
	for (size_t i = 0; i + 8 <= bitBuffer.size(); i += 8)
	{
		string byteStr = bitBuffer.substr(i, 8);
		char byte = static_cast<char>(stoi(byteStr, nullptr, 2)); // Convert binary string to int, then to char
		result.push_back(byte);
	}

	output = result;

#if !DEPLOYED
	cout << endl
		 << endl
		 << "Received message: " << endl
		 << result << endl
		 << endl;
#endif

	stopSampling();
	return 0;
	// exit(0);
}

// Start the demodulation thread, ensuring the PreambleDetector has already stopped
void startDemodulation(string &output)
{
#if !DEPLOYED
	cout << "Starting Demodulation thread..." << endl;
#endif
	demodRunning = true;
	demodThread = thread(demodulationThreadFunc, std::ref(output));
#if !DEPLOYED
	cout << "Demodulation thread started." << endl;
#endif
}

// Stop the demodulation thread and clean up resources
void stopDemodulation()
{
	demodRunning = false;
	demodCond.notify_one();
	if (demodThread.joinable())
	{
		demodThread.join();
	}
}

// Get the current demodulation buffer contents (for debugging)
vector<float> getDemodulationBuffer()
{
	lock_guard<mutex> lock(demodMutex);
	return demodBuffer;
}

// Check if demodulation is active (i.e., the demodulation thread is running)
bool isDemodulationActive()
{
	return demodRunning;
}
