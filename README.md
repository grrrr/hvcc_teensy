# Generator for Teensy audio libraries

## Installation

Needs [hvcc](https://github.com/Wasted-Audio/hvcc) for compilation.

```
python3 -m venv venv
. venv/bin/activate
pip install hvcc_teensy
```

This depends on the [Teensy Audio library](https://github.com/PaulStoffregen/Audio) and on [OpenAudio_ArduinoLibrary](https://github.com/chipaudette/OpenAudio_ArduinoLibrary) for float processing extensions.

The [Pure data](https://puredata.info)-type DSP patch is in `_main.pd`.

Call hvcc-type generator hvcc_teensy with:
```
hvcc _main.pd -n $name -o $dstdir -G hvcc_teensy.hvcc_teensy
```

The Arduino-style library code will be written to `$dstdir` with a header file `$name.h` which can be included in the Arduino program.
