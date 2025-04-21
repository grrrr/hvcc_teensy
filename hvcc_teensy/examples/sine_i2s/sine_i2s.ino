#include <Audio.h>
#include <sine_i2s.h>

sine_i2s heavy;
AudioOutputI2S i2sout;
AudioConnection patchCordL(heavy, 0, i2sout, 0), patchCordR(heavy, 1, i2sout, 1);

void setup() {
  AudioMemory(8);
}

void loop() 
{
  printf("Sample rate: %.0f / Block size: %.0f\n", float(AUDIO_SAMPLE_RATE_EXACT), float(AUDIO_BLOCK_SAMPLES));
  printf("DSP usage: %.2f%% (%.2f%% max)\n", float(AudioProcessorUsage()), float(AudioProcessorUsageMax()));
  delay(1000);
}
