# Resampler
This is a RAII wrapper class for the C library libsamplerate (https://github.com/libsndfile/libsamplerate).

libsamplerate documentation: https://libsndfile.github.io/libsamplerate/api_full.html

## Usage

``` cpp
int channels = 2;
Resampler resampler{channels};

int inputSamples = std::ceil(outputSamples * sampleRateInput / sampleRateOutput);
// fill inputBuffer with inputSamples
resampler.process(inputBuffer, outputBuffer, inputSamples, outputSamples);
```
