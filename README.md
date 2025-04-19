# Generator for Teensy audio libraries

Needs [hvcc](https://github.com/Wasted-Audio/hvcc) for compilation.
This depends on the [Teensy Audio library](https://github.com/PaulStoffregen/Audio) and on [OpenAudio_ArduinoLibrary](https://github.com/chipaudette/OpenAudio_ArduinoLibrary) for float processing extensions.

The [Pure data](https://puredata.info)-type DSP patch is in `_main.pd`.

Call hvcc-stype generator hvcc_teensy with:
```
PYTHONPATH=$PYTHONPATH:$thisrepo hvcc _main.pd -n $name -o $dstdir -G hvcc_teensy.hvcc_teensy
```

The Arduino-style library code will be written to `$dstdir` with a header file `$name.h` which can be included in the Arduino program.
