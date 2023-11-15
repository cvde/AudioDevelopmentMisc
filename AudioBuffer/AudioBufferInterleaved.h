#pragma once

#include <cassert>

namespace edsp
{

template <typename SampleType>
class AudioBufferInterleaved
{
public:
    AudioBufferInterleaved() = default;

    AudioBufferInterleaved(int channels, int samples)
    {
        setSize(channels, samples);
    }

    ~AudioBufferInterleaved()
    {
        deallocateBuffer();
    }

    // disallow copying
    AudioBufferInterleaved(const AudioBufferInterleaved&) = delete;
    AudioBufferInterleaved& operator=(const AudioBufferInterleaved&) = delete;

    // allow moving
    AudioBufferInterleaved(AudioBufferInterleaved&& other)
    {
        mChannels = other.mChannels;
        mSamples = other.mSamples;
        mBuffer = other.mBuffer;
        other.mChannels = 0;
        other.mSamples = 0;
        other.mBuffer = nullptr;
    }
    AudioBufferInterleaved& operator=(AudioBufferInterleaved&& other)
    {
        if (&other == this)
            return *this;

        deallocateBuffer(); // this does not do anything if other is empty

        mChannels = other.mChannels;
        mSamples = other.mSamples;
        mBuffer = other.mBuffer;
        other.mChannels = 0;
        other.mSamples = 0;
        other.mBuffer = nullptr;

        return *this;
    }

    void setSize(int channels, int samples)
    {
        assert(channels > 0 && samples > 0);
        if (channels == mChannels && samples == mSamples)
            return;

        if (mChannels != 0 && mSamples != 0)
            deallocateBuffer();

        mChannels = channels;
        mSamples = samples;
        allocateBuffer();
    }

    const SampleType* getReadPointer() const
    {
        assert(mChannels > 0 && mSamples > 0);
        return mBuffer;
    }

    SampleType* getWritePointer()
    {
        assert(mChannels > 0 && mSamples > 0);
        return mBuffer;
    }

    SampleType getSample(int channel, int sample) const
    {
        assert(mChannels > 0 && mSamples > 0);
        assert(channel >= 0 && channel < mChannels);
        assert(sample >= 0 && sample < mSamples);
        return mBuffer[sample * mChannels + channel];
    }

    SampleType setSample(int channel, int sample, SampleType value)
    {
        assert(mChannels > 0 && mSamples > 0);
        assert(channel >= 0 && channel < mChannels);
        assert(sample >= 0 && sample < mSamples);
        mBuffer[sample * mChannels + channel] = value;
    }

    int getNumChannels() const
    {
        return mChannels;
    }

    int getNumSamples() const
    {
        return mSamples;
    }

private:
    void allocateBuffer()
    {
        const size_t totalSamples = static_cast<size_t>(mChannels) * static_cast<size_t>(mSamples);
        mBuffer = new SampleType[totalSamples];
    }

    void deallocateBuffer()
    {
        if (mChannels != 0 && mSamples != 0)
        {
            if (mBuffer != nullptr)
            {
                delete[] mBuffer;
                mBuffer = nullptr;
            }
        }
    }

    int mChannels = 0;
    int mSamples = 0;
    SampleType* mBuffer = nullptr;
};

} // namespace edsp
