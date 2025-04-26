/** grrrr.org, 2025 */

#ifndef _HvAudioProcessor_hpp
#define _HvAudioProcessor_hpp

#if OPENAUDIO
  #include <AudioStream_F32.h>
  #define AudioStream_CLASS AudioStream_F32
  #define sample_TYPE float32_t
#else
  #include <AudioStream.h>
  #define AudioStream_CLASS AudioStream
  #define sample_TYPE int16_t
  #define SAMPLE_MIN (-(1<<(sizeof(sample_TYPE)*8)))
  #define SAMPLE_MAX ((1<<(sizeof(sample_TYPE)*8))-1)
  #define SAMPLE_SCALE ((float)(SAMPLE_MAX))
#endif

#include "HeavyContext.hpp"

template <class hv_class, int inputs, int outputs> 
class HvAudioProcessor:
  public AudioStream_CLASS
{
public:
  HvAudioProcessor(): 
    AudioStream_CLASS(inputs, inputQueueArray),
    hv_instance(AUDIO_SAMPLE_RATE_EXACT)
  {
    init();
  }

#if OPENAUDIO
  HvAudioProcessor(const AudioSettings_F32 &settings): 
    AudioStream_CLASS(inputs, inputQueueArray),
    hv_instance(settings.sample_rate_Hz)
  {
    init();
  }
#endif

  bool sendmessage(const char *receiver, const HvMessage *msg, double delay=0)
  {
    return hv_instance.sendMessageToReceiver(hv_stringToHash(receiver), delay, msg);
  }

  bool sendmessage(const char *receiver, const char *format, ...)
  {
    hv_assert(format != nullptr);
    double delayMs = 0;

    va_list ap;
    va_start(ap, format);
    const int numElem = (int) hv_strlen(format);
    HvMessage *m = HV_MESSAGE_ON_STACK(numElem);
    msg_init(m, numElem, hv_instance.blockStartTimestamp + (hv_uint32_t) (hv_max_d(0.0, delayMs)*hv_instance.getSampleRate()/1000.0));
    for (int i = 0; i < numElem; i++) {
      switch (format[i]) {
        case 'b': msg_setBang(m, i); break;
        case 'f': msg_setFloat(m, i, (float) va_arg(ap, double)); break;
        case 'h': msg_setHash(m, i, (int) va_arg(ap, int)); break;
        case 's': msg_setSymbol(m, i, (char *) va_arg(ap, char *)); break;
        default: break;
      }
    }
    va_end(ap);

    return sendmessage(receiver, m, delayMs);
  }

  bool sendfloat(const char *receiver, float f)
  {
    return hv_instance.sendFloatToReceiver(hv_stringToHash(receiver), f);
  }

  bool sendbang(const char *receiver)
  {
    return hv_instance.sendBangToReceiver(hv_stringToHash(receiver));
  }

  bool sendsymbol(const char *receiver, const char *s)
  {
    return hv_instance.sendSymbolToReceiver(hv_stringToHash(receiver), s);
  }

protected:

#if OPENAUDIO
  typedef ::audio_block_f32_t audio_block_t;
  audio_block_t *allocate() { return AudioStream_CLASS::allocate_f32(); }
  audio_block_t *receiveReadOnly(unsigned int index = 0) { return AudioStream_CLASS::receiveReadOnly_f32(index); }
  audio_block_t *receiveWritable(unsigned int index = 0) { return AudioStream_CLASS::receiveWritable_f32(index); }
  void transmit(audio_block_t *block, unsigned char index = 0) { AudioStream_CLASS::transmit(block, index); }
  void release(audio_block_t *block) { AudioStream_CLASS::release(block); }
  static int blocklength(const audio_block_t *block) { return block->length; }
#else
  static int blocklength(const audio_block_t *) { return AUDIO_BLOCK_SAMPLES; }
#endif

  void update()
  {
    /*
    - receiveReadOnly() for input data that is not reused/changed for output
    - receiveWritable() for input data that is reused/changed for output
    - allocate() for output data without using input blocks

    - transmit() to pass on audio data
    - release() to release audio data
    */

    audio_block_t *inputBlocks[inputs], *outputBlocks[outputs];
    float *inputArray[inputs], *outputArray[outputs];
    int i;
    bool ok = true;
    int n = AUDIO_BLOCK_SAMPLES;

#if 0
    // ------------------------------
    // safe mode - don't reuse blocks
    // ------------------------------

    for(i = 0; i < inputs; ++i) {
      // we are conservative, don't reuse input audio blocks for output
      inputBlocks[i] = receiveReadOnly(i);
      if(inputBlocks[i] && blocklength(inputBlocks[i]) == n) {
#if OPENAUDIO
        // directly use audio buffer for heavy
        inputArray[i] = inputBlocks[i]->data;
#else
        // use temporary block (int16 type) and scale input
        inputArray[i] = input_tmp[i];
        for(int o = 0; o < n; ++o)
          inputArray[i][o] = float(inputBlocks[i]->data[o])*(1./SAMPLE_SCALE);
#endif
      }
      else
        ok = false;
    }

    for(i = 0; i < outputs; ++i) {
      // for output, we allocate new blocks (don't reuse input buffers)
      outputBlocks[i] = allocate();
      if(outputBlocks[i]) {
#if OPENAUDIO
        // directly use audio buffer for heavy
        outputArray[i] = outputBlocks[i]->data;
#else
        // use temporary block (int16 type)
        outputArray[i] = output_tmp[i];
#endif
      }
      else ok = false;
    }

    if(ok) {
      // perform heavy DSP
      hv_instance.process(inputArray, outputArray, n);

      for(i = 0; i < outputs; ++i) {
#if !(OPENAUDIO)
        // we have to scale heavy output to int16 teensy Audio buffers
        for(int o = 0; o < n; ++o)
          outputBlocks[i]->data[o] = (int)(min(SAMPLE_MAX, max(SAMPLE_MIN, outputArray[i][o]*SAMPLE_SCALE+0.5)));
#endif
        // pass output buffers downstream
        transmit(outputBlocks[i], i);
      }
    }

    // release all input buffers
    for(i = 0; i < inputs; ++i)
      if(inputBlocks[i])
        release(inputBlocks[i]);

    // release all output buffers
    for(i = 0; i < outputs; ++i)
      if(outputBlocks[i])
        release(outputBlocks[i]);

#else
    // ------------------------------
    // reuse audio blocks
    // ------------------------------

    for(i = 0; i < inputs; ++i) {
      // if possible, reuse input blocks for output
      inputBlocks[i] = i < outputs?receiveWritable(i):receiveReadOnly(i);
      if(inputBlocks[i] && blocklength(inputBlocks[i]) == n) {
#if OPENAUDIO
        // directly use audio buffer for heavy
        inputArray[i] = inputBlocks[i]->data;
#else
        // use temporary block (int16 type) and scale input
        // TODO: eventually use arm_math.h functions
        inputArray[i] = input_tmp[i];
        for(int o = 0; o < n; ++o)
          inputArray[i][o] = float(inputBlocks[i]->data[o])*(1./SAMPLE_SCALE);
#endif
      }
      else
        ok = false;
    }

    for(i = 0; i < outputs; ++i) {
      outputBlocks[i] = i < inputs?inputBlocks[i]:allocate();
      if(outputBlocks[i]) {
#if OPENAUDIO
        // directly use audio buffer for heavy
        outputArray[i] = outputBlocks[i]->data;
#else
        outputArray[i] = output_tmp[i];
#endif
      }
      else
        ok = false;
    }

    if(ok) {
      // perform heavy DSP
      hv_instance.process(inputArray, outputArray, n);

      for(i = 0; i < outputs; ++i) {
#if !(OPENAUDIO)
        // we have to scale heavy output to int16 teensy Audio buffers
        // TODO: eventually use arm_math.h functions
        for(int o = 0; o < n; ++o)
          outputBlocks[i]->data[o] = (int)(min(SAMPLE_MAX, max(SAMPLE_MIN, outputArray[i][o]*SAMPLE_SCALE+0.5)));
#endif
        // pass output buffers downstream
        transmit(outputBlocks[i], i);
      }
    }

    // release all input buffers
    for(i = 0; i < inputs; ++i)
      if(inputBlocks[i])
        release(inputBlocks[i]);

    // release all output buffers that have not been used (and released already) as input buffers
    for(; i < outputs; ++i)
      if(outputBlocks[i])
        release(outputBlocks[i]);
#endif
  }

  virtual void receive(double timestampMs, const char *receiverName, const HvMessage *m)
  {
  }

  virtual void print(double timestampMs, const char *receiverName, const char *msgString) 
  {
    printf("[%s] @ %g - %s: %s\n", hv_instance.getName(), timestampMs, receiverName, msgString);
  }

private:
  hv_class hv_instance;

  audio_block_t *inputQueueArray[inputs];

#if !(OPENAUDIO)
  float input_tmp[inputs][AUDIO_BLOCK_SAMPLES];
  float output_tmp[outputs][AUDIO_BLOCK_SAMPLES];
#endif

  void init()
  {
    hv_instance.setUserData(this);
    hv_instance.setPrintHook(printHook);
    hv_instance.setSendHook(sendHook);
  }

  static HvAudioProcessor<hv_class,inputs,outputs> *getThis(HeavyContextInterface *c)
  {
    return static_cast<HvAudioProcessor<hv_class,inputs,outputs> *>(c->getUserData());
  }

  static void printHook(HeavyContextInterface *c, const char *receiverName, const char *msgString, const HvMessage *m)
  {
    double timestampMs = 1000.0 * ((double) ::hv_msg_getTimestamp(m)) / c->getSampleRate();
    getThis(c)->print(timestampMs, receiverName, msgString);
  }

  static void sendHook(HeavyContextInterface *c, const char *receiverName, hv_uint32_t receiverHash, const HvMessage *m)
  {
    double timestampMs = 1000.0 * ((double) ::hv_msg_getTimestamp(m)) / c->getSampleRate();
    getThis(c)->receive(timestampMs, receiverName, m);
  }
};

#endif
