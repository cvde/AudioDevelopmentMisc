#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>

namespace edsp
{

template <typename T>
inline void clearBuffer(T** buffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    const size_t bufferSize = static_cast<size_t>(samples) * sizeof(T);
    for (int channel = 0; channel < channels; ++channel)
        std::memset(buffer[channel], 0.0f, bufferSize);
}

template <typename T>
inline void clearBuffer(T* buffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    const size_t bufferSize = static_cast<size_t>(channels * samples) * sizeof(T);
    std::memset(buffer, 0.0f, bufferSize);
}

template <typename T>
inline void addBuffer(T** sourceBuffer, T** destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[channel][sample] += sourceBuffer[channel][sample];
}

template <typename T>
inline void addBuffer(T* sourceBuffer, T* destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int i = 0; i < channels * samples; ++i)
        destinationBuffer[i] += sourceBuffer[i];
}

template <typename T>
inline void interleaveSamples(T** sourceBuffer, T* destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[sample * channels + channel] = sourceBuffer[channel][sample];
}

template <typename T>
inline void deinterleaveSamples(T* sourceBuffer, T** destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[channel][sample] = sourceBuffer[channels * sample + channel];
}

} // namespace edsp
