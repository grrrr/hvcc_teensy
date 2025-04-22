#include <Audio.h>
#include <sine_out.h>

sine_out heavy;
AudioOutputI2S i2sout;
AudioConnection patchCordL(heavy, 0, i2sout, 0), patchCordR(heavy, 1, i2sout, 1);

void setup() {
  AudioMemory(8);
}

void loop() 
{
  printf("Sample rate: %.0f / Block size: %.0f\n", float(AUDIO_SAMPLE_RATE_EXACT), float(AUDIO_BLOCK_SAMPLES));
  printf("DSP usage: %.2f%% (%.2f%% max)\n", float(AudioProcessorUsage()), float(AudioProcessorUsageMax()));
  printf("Mem usage: %d blocks (%d blocks max)\n", int(AudioMemoryUsage()), int(AudioMemoryUsageMax()));
  delay(1000);
}
