#ifndef _HvAudioProcessor_hpp
#define _HvAudioProcessor_hpp

#include <AudioStream_F32.h>
#include "HeavyContext.hpp"

template <class hv_class, int inputs, int outputs> 
class HvAudioProcessor:
  public AudioStream_F32
{
public:
  HvAudioProcessor(): 
    AudioStream_F32(inputs, inputQueueArray_f32),
    hv_instance(AUDIO_SAMPLE_RATE_EXACT)
  {
    init();
  }

  HvAudioProcessor(const AudioSettings_F32 &settings): 
    AudioStream_F32(inputs, inputQueueArray_f32),
    hv_instance(AUDIO_SAMPLE_RATE_EXACT)
  {
    init();
  }

protected:
  void update()
  {
    audio_block_f32_t *outputBlocks[outputs];
    int i;
    bool ok = true;
    int n = 0;
    for(i = 0; i < outputs; i++) {
      outputBlocks[i] = AudioStream_F32::receiveWritable_f32(i);
      if(!outputBlocks[i]) 
        ok = false;
      else
        outputArray[i] = outputBlocks[i]->data;
      if(n != outputBlocks[i]->length) {
        if(n)
          ok = false;
        else
          n = outputBlocks[i]->length;
      }
    }

    if(!ok) goto release;

    hv_instance.process(inputArray, outputArray, n);

    for(i = 0; i < outputs; ++i)
      AudioStream_F32::transmit(outputBlocks[i], i);

    release:
    for(i = 0; i < outputs; ++i)
      AudioStream_F32::release(outputBlocks[i]);
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

  virtual void receive(double timestamp, const char *receiverName, const HvMessage *m) 
  {
  }

  virtual void print(double timestamp, const char *receiverName, const char *msgString) 
  {
    printf("[%s] @ %g - %s: %s\n", hv_instance.getName(), timestamp, receiverName, msgString);
  }

private:
  hv_class hv_instance;

  float32_t *inputArray[inputs];
  float32_t *outputArray[outputs];

  audio_block_f32_t *inputQueueArray_f32[inputs];

  void init()
  {
    hv_instance.setUserData(this);
    for(int i = 0; i < inputs; ++i)
      inputArray[i] = inputQueueArray_f32[i]->data;
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
    if(!strcmp(receiverName, "HV_TO_PD")) {
      double timestampMs = 1000.0 * ((double) ::hv_msg_getTimestamp(m)) / c->getSampleRate();
      getThis(c)->receive(timestampMs, receiverName, m);
    }
  }
};

#endif
