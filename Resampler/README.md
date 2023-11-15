# Resampler
This is a RAII wrapper class for the C library libsamplerate (https://github.com/libsndfile/libsamplerate).

libsamplerate documentation: https://libsndfile.github.io/libsamplerate/api_full.html

## Usage

``` cpp
#include "Resampler/Resampler.h"

int channels = 2;
int outputSamples = 512;
double sampleRateInput = 48000.0;
double sampleRateOutput = 44100.0;

// call e.g. at application start
edsp::Resampler resampler{channels};

// calculate how many samples to put in for a given amount of output samples
int inputSamples = std::ceil(outputSamples * sampleRateInput / sampleRateOutput);

// call from audio thread
resampler.process(inputInterleavedAudioBuffer, outputInterleavedAudioBuffer, inputSamples, outputSamples);
```
