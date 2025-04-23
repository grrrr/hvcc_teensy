#include <OpenAudio_ArduinoLibrary.h>

#define OPENAUDIO 1
#include <crossover_noise_f32.h>

AudioSynthNoiseWhite_F32 noise;
crossover_noise_f32 xover;
AudioOutputI2S_F32 i2sout;
AudioConnection_F32 patchCordIn(noise, 0, xover, 0);
AudioConnection_F32 patchCordLP(xover, 0, i2sout, 0), patchCordHP(xover, 1, i2sout, 1);

void setup()
{
  AudioMemory_F32(8);

  noise.amplitude(0.1);
  xover.sendfloat("frq", 2000);
  xover.sendfloat("hf_att", -10);
  xover.sendfloat("lf_del", 0.2);
}

void loop() 
{
  printf("Sample rate: %.0f / Block size: %.0f\n", float(AUDIO_SAMPLE_RATE_EXACT), float(AUDIO_BLOCK_SAMPLES));
  printf("DSP usage: %.2f%% (%.2f%% max)\n", float(AudioProcessorUsage()), float(AudioProcessorUsageMax()));
  printf("Mem usage: %d blocks (%d blocks max)\n", int(AudioMemoryUsage_F32()), int(AudioMemoryUsageMax_F32()));
  delay(1000);
}
