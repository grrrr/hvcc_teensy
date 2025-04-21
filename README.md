# Generator for Teensy audio libraries

## Installation

Needs [hvcc](https://github.com/Wasted-Audio/hvcc) for compilation.

```
python3 -m venv venv
. venv/bin/activate
pip install hvcc_teensy
```

## Generation

The following translates the Puredata patch `_main.pd` into C++ files in a folder `$DST`.
The object ("context") will also be called `$NAME`:

```
hvcc _main.pd -n $NAME -o $DST -G hvcc_teensy
```

## Usage

The contents of the subfolder `$DST/teensy` represent library code usable in the Arduino environment.
There are (potential) standard locations for these libraries:

- MacOS/Linux: `$HOME/Arduino/libraries/`
- Windows: `My Documents\Arduino\libraries`

In the generated library folder will be a header file `$NAME.h` to be included in the Arduino program.

The produced code depends on the [Teensy Audio library](https://github.com/PaulStoffregen/Audio).
If `OPENAUDIO` is `#define`d before including the `$NAME.h` header file, the code will depend on [OpenAudio_ArduinoLibrary](https://github.com/chipaudette/OpenAudio_ArduinoLibrary) for float_32 processing extensions.
