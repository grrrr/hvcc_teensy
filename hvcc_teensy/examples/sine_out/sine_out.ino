#include <sine_out.h>
sine_out sine;

#if OPENAUDIO
#include <OpenAudio_ArduinoLibrary.h>

AudioOutputI2S_F32 i2sout;
AudioConnection_F32 patchCordL(sine, 0, i2sout, 0), patchCordR(sine, 0, i2sout, 1);

#else
#include <Audio.h>

AudioOutputI2S i2sout;
AudioConnection patchCordL(sine, 0, i2sout, 0), patchCordR(sine, 0, i2sout, 1);

#endif

void setup() {
  AudioMemory(8);
#if OPENAUDIO
  AudioMemory_F32(8);
#endif

  sine.sendfloat("frq", 200);
  sine.sendfloat("vol", -20);
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
