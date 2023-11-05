# Resampler
This is a RAII wrapper class for the C library libsamplerate (https://github.com/libsndfile/libsamplerate).

libsamplerate documentation: https://libsndfile.github.io/libsamplerate/api_full.html

## Usage

``` cpp
Resampler resampler;

const int numInputSamples = std::ceil(numOutputSamples * sampleRateInput / sampleRateOutput);
// fill inputBuffer with numInputSamples
resampler.process(inputBuffer, outputBuffer, numInputSamples, numOutputSamples);
```
