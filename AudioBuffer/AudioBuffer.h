#pragma once

#include <cassert>

template <typename T, int MAX_CHANNELS>
class AudioBuffer
{
public:
    AudioBuffer() = default;

    AudioBuffer(int samples, int channels)
    {
        setSize(samples, channels);
    }

    ~AudioBuffer()
    {
        deallocateBuffer();
    }

    // disallow copying
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;

    // allow moving
    AudioBuffer(AudioBuffer&& other)
    {
        mSamples = other.mSamples;
        mChannels = other.mChannels;
        for (int channel = 0; channel < mChannels; ++channel)
        {
            mBuffer[channel] = other.mBuffer[channel];
            other.mBuffer[channel] = nullptr;
        }
        other.mSamples = 0;
        other.mChannels = 0;
    }

    AudioBuffer& operator=(AudioBuffer&& other)
    {
        if (&other == this)
            return *this;

        deallocateBuffer();

        mSamples = other.mSamples;
        mChannels = other.mChannels;
        for (int channel = 0; channel < mChannels; ++channel)
        {
            mBuffer[channel] = other.mBuffer[channel];
            other.mBuffer[channel] = nullptr;
        }
        other.mSamples = 0;
        other.mChannels = 0;

        return *this;
    }

    void setSize(int samples, int channels)
    {
        assert(samples > 0 && channels > 0);
        assert(channels <= MAX_CHANNELS);
        if (samples == mSamples && channels == mChannels)
            return;

        if (mSamples != 0 && mChannels != 0)
        {
            deallocateBuffer();
        }

        mSamples = samples;
        mChannels = channels;
        allocateBuffer();
    }

    T** getArrayOfWritePointers()
    {
        assert(mSamples > 0 && mChannels > 0);
        return mBuffer;
    }

    T* getWritePointer(int channel)
    {
        assert(mSamples > 0 && mChannels > 0);
        assert(channel <= mChannels);
        return mBuffer[channel];
    }

    int getNumSamples() const
    {
        return mSamples;
    }

    int getNumChannels() const
    {
        return mChannels;
    }

private:
    void allocateBuffer()
    {
        for (int channel = 0; channel < mChannels; ++channel)
        {
            mBuffer[channel] = new T[static_cast<size_t>(mSamples)];
        }
    }

    void deallocateBuffer()
    {
        if (mSamples != 0 && mChannels != 0)
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

    int mSamples = 0;
    int mChannels = 0;
    T* mBuffer[MAX_CHANNELS] = nullptr;
};
