#include <crossover_thru.h>
crossover_thru xover;

#if OPENAUDIO
#include <OpenAudio_ArduinoLibrary.h>

AudioInputI2S_F32 i2sin;
AudioOutputI2S_F32 i2sout;
AudioConnection_F32 patch_in(i2sin, 0, xover, 0);
AudioConnection_F32 patch_lp(xover, 0, i2sout, 0), patch_hp(xover, 1, i2sout, 1);

#else
#include <Audio.h>

AudioInputI2S i2sin;
AudioOutputI2S i2sout;
AudioConnection patch_in(i2sin, 0, xover, 0);
AudioConnection patch_lp(xover, 0, i2sout, 0), patch_hp(xover, 1, i2sout, 1);

#endif

void setup()
{
  AudioMemory(8);
#if OPENAUDIO
  AudioMemory_F32(8);
#endif

  xover.sendfloat("frq", 2000);
  xover.sendfloat("hf_att", -10);
  xover.sendfloat("lf_del", 0.2);
}

void loop() 
{
  printf("Sample rate: %.0f / Block size: %.0f\n", float(AUDIO_SAMPLE_RATE_EXACT), float(AUDIO_BLOCK_SAMPLES));
  printf("DSP usage: %.2f%% (%.2f%% max)\n", float(AudioProcessorUsage()), float(AudioProcessorUsageMax()));
  printf("Mem usage: %d blocks", int(AudioMemoryUsageMax()));
#if OPENAUDIO
  printf(" / Mem_F32 usage: %d blocks\n", int(AudioMemoryUsageMax_F32()));
#else
  printf("\n");
#endif
  delay(1000);
}
