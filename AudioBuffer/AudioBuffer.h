// SPDX-FileCopyrightText: 2023 Christian Voigt
// SPDX-License-Identifier: MIT

#pragma once

#include <cassert>
#include <cstddef>

namespace edsp
{

template <typename SampleType, int MAX_CHANNELS>
class AudioBuffer
{
public:
    AudioBuffer() = default;

    AudioBuffer(int channels, int samples)
    {
        setSize(channels, samples);
    }

    ~AudioBuffer() noexcept
    {
        deallocateBuffer();
    }

    // disallow copying
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;

    // allow moving
    AudioBuffer(AudioBuffer&& other) noexcept
    {
        mChannels = other.mChannels;
        mSamples = other.mSamples;
        for (int channel = 0; channel < mChannels; ++channel)
        {
            mBuffer[channel] = other.mBuffer[channel];
            other.mBuffer[channel] = nullptr;
        }
        other.mChannels = 0;
        other.mSamples = 0;
    }
    AudioBuffer& operator=(AudioBuffer&& other) noexcept
    {
        if (&other == this)
            return *this;

        deallocateBuffer(); // this does not do anything if other is empty

        mChannels = other.mChannels;
        mSamples = other.mSamples;
        for (int channel = 0; channel < mChannels; ++channel)
        {
            mBuffer[channel] = other.mBuffer[channel];
            other.mBuffer[channel] = nullptr;
        }
        other.mChannels = 0;
        other.mSamples = 0;

        return *this;
    }

    void setSize(int channels, int samples)
    {
        assert(channels <= MAX_CHANNELS);
        assert(channels > 0 && samples > 0);
        if (channels == mChannels && samples == mSamples)
            return;

        if (mChannels != 0 && mSamples != 0)
            deallocateBuffer();

        mChannels = channels;
        mSamples = samples;
        allocateBuffer();
    }

    const SampleType** getArrayOfReadPointers() const noexcept
    {
        assert(mChannels > 0 && mSamples > 0);
        return mBuffer;
    }

    SampleType** getArrayOfWritePointers() noexcept
    {
        assert(mChannels > 0 && mSamples > 0);
        return mBuffer;
    }

    const SampleType* getReadPointer(int channel) const noexcept
    {
        assert(mChannels > 0 && mSamples > 0);
        assert(channel < mChannels);
        return mBuffer[channel];
    }

    SampleType* getWritePointer(int channel) noexcept
    {
        assert(mChannels > 0 && mSamples > 0);
        assert(channel < mChannels);
        return mBuffer[channel];
    }

    SampleType getSample(int channel, int sample) const noexcept
    {
        assert(mChannels > 0 && mSamples > 0);
        assert(channel >= 0 && channel < mChannels);
        assert(sample >= 0 && sample < mSamples);
        return mBuffer[channel][sample];
    }

    SampleType setSample(int channel, int sample, SampleType value) noexcept
    {
        assert(mChannels > 0 && mSamples > 0);
        assert(channel >= 0 && channel < mChannels);
        assert(sample >= 0 && sample < mSamples);
        mBuffer[channel][sample] = value;
    }

    int getNumChannels() const noexcept
    {
        return mChannels;
    }

    int getNumSamples() const noexcept
    {
        return mSamples;
    }

private:
    void allocateBuffer()
    {
        for (int channel = 0; channel < mChannels; ++channel)
            mBuffer[channel] = new SampleType[static_cast<std::size_t>(mSamples)];
    }

    void deallocateBuffer() noexcept
    {
        if (mChannels != 0 && mSamples != 0)
        {
            for (int channel = 0; channel < mChannels; ++channel)
            {
                if (mBuffer[channel] != nullptr)
                {
                    delete[] mBuffer[channel];
                    mBuffer[channel] = nullptr;
                }
            }
        }
    }

    int mChannels = 0;
    int mSamples = 0;
    SampleType* mBuffer[MAX_CHANNELS] = {};
};

} // namespace edsp
