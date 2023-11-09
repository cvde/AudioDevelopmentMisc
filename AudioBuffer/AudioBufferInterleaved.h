#pragma once

#include <cassert>

template <typename T>
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

    T* getWritePointer()
    {
        assert(mChannels > 0 && mSamples > 0);
        return mBuffer;
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
        mBuffer = new T[totalSamples];
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
    T* mBuffer = nullptr;
};
