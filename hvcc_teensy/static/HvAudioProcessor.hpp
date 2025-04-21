#ifndef _HvAudioProcessor_hpp
#define _HvAudioProcessor_hpp

#ifdef OPENAUDIO
  #include <AudioStream_F32.h>
  #define AudioStream_CLASS AudioStream_F32
  #define audio_block_TYPE audio_block_f32_t
  #define sample_TYPE float32_t
#else
  #include <AudioStream.h>
  #define AudioStream_CLASS AudioStream
  #define audio_block_TYPE audio_block_t
  #define sample_TYPE int16_t
  #define SAMPLE_MIN (-(1<<(sizeof(sample_TYPE)*8))
  #define SAMPLE_MAX ((1<<(sizeof(sample_TYPE)*8))-1)
  #define SAMPLE_SCALE (float(SAMPLE_MAX))
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

#ifdef OPENAUDIO
  HvAudioProcessor(const AudioSettings_F32 &settings): 
    AudioStream_CLASS(inputs, inputQueueArray),
    hv_instance(settings.sample_rate_Hz)
  {
    init();
  }
#endif

protected:
  void update()
  {
    audio_block_TYPE *outputBlocks[outputs];
    int i;
    bool ok = true;
    int n = 0;
    for(i = 0; i < outputs; i++) {
#ifdef OPENAUDIO
      outputBlocks[i] = AudioStream_CLASS::receiveWritable_f32(i);
#else
      outputBlocks[i] = AudioStream_CLASS::receiveWritable(i);
#endif
      if(!outputBlocks[i]) 
        ok = false;
#ifdef OPENAUDIO
      else
        outputArray[i] = outputBlocks[i]->data;
#endif
      if(n != outputBlocks[i]->length) {
        if(n)
          ok = false;
        else
          n = outputBlocks[i]->length;
      }
    }

    if(!ok) goto release;

#ifndef OPENAUDIO
    // we need to copy and scale data
    for(i = 0; i < inputs; ++i) {
      for(int o = 0; o < n; ++o)
      inputArray[i][o] = float(inputQueueArray[i]->data[o])*(1./SAMPLE_SCALE);
    }
#endif

    hv_instance.process(inputArray, outputArray, n);

    for(i = 0; i < outputs; ++i) {
#ifndef OPENAUDIO
      // we need to copy and scale data
      for(int o = 0; o < n; ++o)
        outputBlocks[i]->data[o] = int(min(max(SAMPLE_MIN, outputArray[i][o]*SAMPLE_SCALE+0.5), SAMPLE_MAX));
#endif
      AudioStream_CLASS::transmit(outputBlocks[i], i);
    }

    release:
    for(i = 0; i < outputs; ++i)
      AudioStream_CLASS::release(outputBlocks[i]);
  }

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

  virtual void receive(double timestampMs, const char *receiverName, const HvMessage *m) 
  {
  }

  virtual void print(double timestampMs, const char *receiverName, const char *msgString) 
  {
    printf("[%s] @ %g - %s: %s\n", hv_instance.getName(), timestampMs, receiverName, msgString);
  }

private:
  hv_class hv_instance;

#ifdef OPENAUDIO
  float *inputArray[inputs];
  float *outputArray[outputs];
#else
  float inputArray[inputs][AUDIO_BLOCK_SIZE];
  float outputArray[outputs][AUDIO_BLOCK_SIZE];
#endif

  audio_block_TYPE *inputQueueArray[inputs];

  void init()
  {
    hv_instance.setUserData(this);
#ifdef OPENAUDIO
    for(int i = 0; i < inputs; ++i)
      inputArray[i] = inputQueueArray[i]->data;
#endif
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

  static void sendHook(HeavyContextInterface *c, const char *receiverName, unsigned int receiverHash, const HvMessage *m)
  {
    if(!strcmp(receiverName, "HV_TO_TEENSY")) {
      double timestampMs = 1000.0 * ((double) ::hv_msg_getTimestamp(m)) / c->getSampleRate();
      getThis(c)->receive(timestampMs, receiverName, m);
    }
  }
};

#endif
