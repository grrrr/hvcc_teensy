#include <OpenAudio_ArduinoLibrary.h>
#include <hexefx_audiolib_F32.h>

#define OPENAUDIO
#include <crossover_f32.h>

AudioInputI2S_F32 i2sin;
crossover_f32 xover;
AudioOutputI2S_F32 i2sout;
AudioConnection_F32 patchCordIn(i2sin, 0, xover, 0);
AudioConnection_F32 patchCordLP(xover, 0, i2sout, 0), patchCordHP(xover, 1, i2sout, 1);
AudioControlWM8731_F32 codec;

static bool ok = false;

void setup() {
  delay(1000); // allow codec to awake
  ok = codec.enable();
  if(ok) {
    AudioMemory_F32(64);
    codec.volume(1);

    codec.inputSelect(AUDIO_INPUT_LINEIN);
    codec.lineIn_mute(false);
    codec.inputLevel(0.62);
    codec.hp_filter(true);
  }
}

void loop() 
{
  if(ok) {
    printf("Sample rate: %.0f / Block size: %.0f\n", float(AUDIO_SAMPLE_RATE_EXACT), float(AUDIO_BLOCK_SAMPLES));
    printf("DSP usage: %.2f%% (%.2f%% max)\n", float(AudioProcessorUsage()), float(AudioProcessorUsageMax()));
	  printf("Mem usage: %d blocks (%d blocks max)\n", int(AudioMemoryUsage_F32()), int(AudioMemoryUsageMax_F32()));
  }
  else
    printf("Codec init FAILED\n");

  delay(1000);
}
