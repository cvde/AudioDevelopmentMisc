#pragma once

#include <cassert>
#include <cstring>

inline void clearBuffer(float** buffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    const size_t bufferSize = static_cast<size_t>(samples) * sizeof(float);
    for (int channel = 0; channel < channels; ++channel)
        std::memset(buffer[channel], 0.0f, bufferSize);
}

inline void clearBuffer(float* buffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    const size_t bufferSize = static_cast<size_t>(samples * channels) * sizeof(float);
    std::memset(buffer, 0.0f, bufferSize);
}

inline void addBuffer(float** sourceBuffer, float** destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[channel][sample] += sourceBuffer[channel][sample];
}

inline void addBuffer(float* sourceBuffer, float* destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int i = 0; i < samples * channels; ++i)
        destinationBuffer[i] += sourceBuffer[i];
}

inline void interleaveSamples(float** sourceBuffer, float* destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[sample * channels + channel] = sourceBuffer[channel][sample];
}

inline void deinterleaveSamples(float* sourceBuffer, float** destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[channel][sample] = sourceBuffer[sample * channels + channel];
}
