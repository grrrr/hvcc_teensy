{{copyright}}

#ifndef HeavyTeensy_{{name}}_h
#define HeavyTeensy_{{name}}_h

{{defines}}

#include "Heavy_{{name}}.h"
#include "Heavy_{{name}}.hpp"

#include "HvAudioProcessor.hpp"

class {{struct_name}}:
  public HvAudioProcessor<Heavy_{{name}}, {{num_input_channels}}, {{num_output_channels}}>
{};

#endif
