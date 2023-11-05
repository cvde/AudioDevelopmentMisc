#pragma once

#include <cassert>

template <typename T>
class AudioBufferInterleaved
{
public:
    AudioBufferInterleaved() = default;

    AudioBufferInterleaved(int samples, int channels)
    {
        setSize(samples, channels);
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
        mSamples = other.mSamples;
        mChannels = other.mChannels;
        mBuffer = other.mBuffer;
        other.mSamples = 0;
        other.mChannels = 0;
        other.mBuffer = nullptr;
    }
    AudioBufferInterleaved& operator=(AudioBufferInterleaved&& other)
    {
        if (&other == this)
            return *this;

        deallocateBuffer();

        mSamples = other.mSamples;
        mChannels = other.mChannels;
        mBuffer = other.mBuffer;
        other.mSamples = 0;
        other.mChannels = 0;
        other.mBuffer = nullptr;

        return *this;
    }

    void setSize(int samples, int channels)
    {
        assert(samples > 0 && channels > 0);
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

    T* getWritePointer()
    {
        assert(mSamples > 0 && mChannels > 0);
        return mBuffer;
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
        const size_t totalSamples = static_cast<size_t>(mSamples) * static_cast<size_t>(mChannels);
        mBuffer = new T[totalSamples];
    }

    void deallocateBuffer()
    {
        if (mSamples != 0 && mChannels != 0)
        {
            if (mBuffer != nullptr)
            {
                delete[] mBuffer;
                mBuffer = nullptr;
            }
        }
    }

    int mSamples = 0;
    int mChannels = 0;
    T* mBuffer = nullptr;
};
