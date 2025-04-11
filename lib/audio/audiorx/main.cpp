#if false

#include "AudioDevice.h"
#include "Sampling.h"
#include "PreambleDetector.h"
#include <thread>
#include <atomic>
using namespace std;

atomic<bool> preambleDetected(false);

int main() {
	snd_pcm_t* pcm_handle = nullptr;
	if (!AudioDevice::init(pcm_handle)) {
		return -1;
	}

	std::cout << "Starting threads" << std::endl;
	thread samplingThread(samplingThreadFunc, pcm_handle);
	thread preambleThread(run);

	samplingThread.join();
	preambleThread.join();

	AudioDevice::close(pcm_handle);
	return 0;
}

#endif