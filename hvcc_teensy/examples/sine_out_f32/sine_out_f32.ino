#include <OpenAudio_ArduinoLibrary.h>

#include <sine_out_f32.h>

sine_out_f32 heavy;
AudioOutputI2S_F32 i2sout;
AudioConnection_F32 patchCordL(heavy, 0, i2sout, 0), patchCordR(heavy, 1, i2sout, 1);

void setup() 
{
  AudioMemory_F32(8);
}

void loop() 
{
  printf("Sample rate: %.0f / Block size: %.0f\n", float(AUDIO_SAMPLE_RATE_EXACT), float(AUDIO_BLOCK_SAMPLES));
  printf("DSP usage: %.2f%% (%.2f%% max)\n", float(AudioProcessorUsage()), float(AudioProcessorUsageMax()));
  printf("Mem usage: %d blocks (%d blocks max)\n", int(AudioMemoryUsage_F32()), int(AudioMemoryUsageMax_F32()));
  delay(1000);
}
