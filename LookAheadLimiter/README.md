# LookAheadLimiter
This implementation is based on https://github.com/tcarpent/PeakLimiter.

The following example limits the signal to -3 dBFS and then adds 2 dB (the output peaks at -1 dBFS).

## Usage

``` cpp
#include "LookAheadLimiter/LookAheadLimiter.h"

float attackMs = 10.0f;
float releaseMs = 50.0f;
float thresholdInDb = -3.0f;
float makeupGainInDb = 2.0f;
int channels = 2;
double sampleRate = 48000.0;

edsp::LookAheadLimiter limiter;

// call e.g. at application start
limiter.configure(attackMs, releaseMs, thresholdInDb, makeupGainInDb, channels, sampleRate);

// call from audio thread
limiter.process(interleavedAudioBuffer, samples);
```
