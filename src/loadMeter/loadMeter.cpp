#include "../../Jaffx.hpp"

struct LoadMeter : Jaffx::Program {
  CpuLoadMeter loadMeter;

  void init() override {
    hardware.StartLog(true);
    loadMeter.Init(hardware.AudioSampleRate(), hardware.AudioBlockSize());
  }

  float processAudio(float in) override {
    return 0.f;
  }

  void blockStart() override {
    loadMeter.OnBlockStart();
  }
  void blockEnd() override {
    loadMeter.OnBlockEnd();
  }

  void loop() override {
    const float avgLoad = loadMeter.GetAvgCpuLoad();
    const float maxLoad = loadMeter.GetMaxCpuLoad();
    const float minLoad = loadMeter.GetMinCpuLoad();
    // print it to the serial connection (as percentages)
    hardware.PrintLine("Processing Load:");
    hardware.PrintLine("Max: " FLT_FMT3 "%%", FLT_VAR3(maxLoad * 100.0f));
    hardware.PrintLine("Avg: " FLT_FMT3 "%%", FLT_VAR3(avgLoad * 100.0f));
    hardware.PrintLine("Min: " FLT_FMT3 "%%", FLT_VAR3(minLoad * 100.0f));
    // don't spam the serial connection too much
    System::Delay(1000);
  }

};

int main() {
  LoadMeter mLoadMeter;
  mLoadMeter.start();
  return 0;
}


