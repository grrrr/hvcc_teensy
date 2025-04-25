#include <crossover_noise.h>

class _crossover_noise:
  public crossover_noise
{
protected:
  virtual void receive(double timestampMs, const char *receiverName, const HvMessage *m) 
  {
    return;
    if(!strcmp(receiverName, "bang")) {
        printf("BANG @ %g\n", timestampMs);
    }
    else if(!strcmp(receiverName, "frq") && msg_isFloat(m, 0)) {
      float f = msg_getFloat(m, 0);
      printf("RECV @ %g - %s %f\n", timestampMs, receiverName, f);
    }
  }
} xover;


#if OPENAUDIO
#include <OpenAudio_ArduinoLibrary.h>

AudioSynthNoiseWhite_F32 noise;
AudioConnection_F32 patch_in(noise, 0, xover, 0);
AudioInputAnalog inp_frq(A0, false);
AudioConvert_I16toF32 cnv_frq;
AudioConnection conn_frq(inp_frq, 0, cnv_frq, 0);
AudioConnection_F32 patch_frq(cnv_frq, 0, xover, 1);
AudioOutputI2S_F32 i2sout;
AudioConnection_F32 patch_lp(xover, 0, i2sout, 0), patch_hp(xover, 1, i2sout, 1);

#else
#include <Audio.h>

AudioSynthNoiseWhite noise;
AudioConnection patch_in(noise, 0, xover, 0);
AudioInputAnalog inp_frq(A0, false);
AudioConnection conn_frq(inp_frq, 0, xover, 1);
AudioOutputI2S i2sout;
AudioConnection patch_lp(xover, 0, i2sout, 0), patch_hp(xover, 1, i2sout, 1);

#endif

void setup()
{
  AudioMemory(8);
#if OPENAUDIO
  AudioMemory_F32(8);
#endif

  noise.amplitude(0.1);
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
