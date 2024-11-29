#include "libDaisy/src/daisy_seed.h"
#include "JaffMalloc.hpp"
using namespace daisy;


Jaffx::MyMalloc m;
namespace giml {
	//Overwrite stdlib calls in giml space
	void* malloc(size_t size) {
		//std::cout << "MyMalloc" << std::endl;
		return m.malloc(size);
	}

	void* calloc(size_t nelemb, size_t size) {
		//std::cout << "MyCalloc" << std::endl;
		return m.calloc(nelemb, size);
	}

	void* realloc(void* ptr, size_t size) {
		//std::cout << "MyRealloc" << std::endl;
		return m.realloc(ptr, size);
	}

	void free(void* ptr) {
		//std::cout << "MyFree" << std::endl;
		m.free(ptr);
	}
}


// //Overwrite stdlib calls in global space
// Jaffx::MyMalloc m;
// void* malloc(size_t size) {
// 	return m.malloc(size);
// }

// void* calloc(size_t nelemb, size_t size) {
// 	return m.calloc(nelemb, size);
// }

// void *realloc(void *ptr, size_t size) {
// 	return m.realloc(ptr, size);
// }

// void free(void* ptr) {
// 	m.free(ptr);
// }

namespace Jaffx {
	struct Program {
		// declare an instance of the hardware
		static DaisySeed hardware;
		static Program* instance; // Static pointer to the current instance of Program

		// It's handy to have these numbers on tap
		const int samplerate = 48000;
		const int buffersize = 128;

		// loadMeter for debugging, bool for toggle
		CpuLoadMeter loadMeter;
		bool debug = false;

		// overridable init function
		inline virtual void init() {}

		// debug init function
		inline void initDebug() {
			if (debug) {
				hardware.StartLog();
				loadMeter.Init(hardware.AudioSampleRate(), hardware.AudioBlockSize());
			}
		}

		// overridable per-sample operation
		inline virtual float processAudio(float in) { return in; }

		// overridable loop operation
		inline virtual void loop() {}

		// debug loop
		inline void debugLoop() {
			if (debug) {
				// as seen in https://electro-smith.github.io/libDaisy/md_doc_2md_2__a3___getting-_started-_audio.html
				const float avgLoad = loadMeter.GetAvgCpuLoad();
				const float maxLoad = loadMeter.GetMaxCpuLoad();
				const float minLoad = loadMeter.GetMinCpuLoad();
				// print it to the serial connection (as percentages)
				hardware.PrintLine("Processing Load:");
				hardware.PrintLine("Max: " FLT_FMT3 "%%", FLT_VAR3(maxLoad * 100.0f));
				hardware.PrintLine("Avg: " FLT_FMT3 "%%", FLT_VAR3(avgLoad * 100.0f));
				hardware.PrintLine("Min: " FLT_FMT3 "%%", FLT_VAR3(minLoad * 100.0f));
				System::Delay(1000); // Don't spam the serial!
			}
		}

		// overridable block start/end operation
		inline virtual void blockStart() {}
		inline virtual void blockEnd() {}

		// basic mono->dual-mono callback
		static void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
			if (instance->debug) {
				instance->loadMeter.OnBlockStart();
			}
			instance->blockStart();
			for (size_t i = 0; i < size; i++) {
				out[0][i] = instance->processAudio(in[0][i]); // format is in/out[channel][sample]
				out[1][i] = out[0][i];
			}
			instance->blockEnd();
			if (instance->debug) {
				instance->loadMeter.OnBlockEnd();
			}
		}

		void start() {
			// initialize hardware
			hardware.Init();
			hardware.SetAudioBlockSize(buffersize); // number of samples handled per callback (buffer size)
			hardware.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ); // sample rate

			m.MyMallocInit(); //Needs to be called AFTER hardware init, and not in the object's constructor

			// init instance and start callback
			instance = this;
			this->init();
			this->initDebug();
			hardware.StartAudio(AudioCallback);

			// loop indefinitely
			while (1) {
				this->loop(); this->debugLoop();
			}
		}
		
	};

	// Global instancing of static members
	DaisySeed Program::hardware; 
	Program* Program::instance = nullptr;

} // namespace Jaffx