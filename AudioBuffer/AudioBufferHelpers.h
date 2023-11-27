// SPDX-FileCopyrightText: 2023 Christian Voigt
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>

namespace edsp
{

template <typename SampleType>
inline void clearBuffer(SampleType** buffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    const std::size_t bufferSize = static_cast<std::size_t>(samples) * sizeof(SampleType);
    for (int channel = 0; channel < channels; ++channel)
        std::memset(buffer[channel], 0.0f, bufferSize);
}

template <typename SampleType>
inline void clearBuffer(SampleType* buffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    const std::size_t bufferSize = static_cast<std::size_t>(channels * samples) * sizeof(SampleType);
    std::memset(buffer, 0.0f, bufferSize);
}

template <typename SampleType>
inline void addBuffer(const SampleType** sourceBuffer, SampleType** destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[channel][sample] += sourceBuffer[channel][sample];
}

template <typename SampleType>
inline void addBuffer(const SampleType* sourceBuffer, SampleType* destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int i = 0; i < channels * samples; ++i)
        destinationBuffer[i] += sourceBuffer[i];
}

template <typename SampleType>
inline void interleaveSamples(const SampleType** sourceBuffer, SampleType* destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            destinationBuffer[sample * channels + channel] = sourceBuffer[channel][sample];
}

template <typename SampleType>
inline void deinterleaveSamples(const SampleType* sourceBuffer, SampleType** destinationBuffer, int channels, int samples)
{
    assert(channels > 0);
    assert(samples > 0);

    for (int sample = 0; sample < samples; ++sample)
        for (int channel = 0; channel < channels; ++channel)
            destinationBuffer[channel][sample] = sourceBuffer[channels * sample + channel];
}

} // namespace edsp
