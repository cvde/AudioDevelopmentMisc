#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

namespace edsp
{

class LookAheadLimiter
{
public:
    LookAheadLimiter() = default;

    // configure the limiter
    void configure(float attackMs,       // attack/lookahead time in milliseconds
                   float releaseMs,      // release time in milliseconds
                   float thresholdInDb,  // limiting threshold in dBFS
                   float makeupGainInDb, // makeup gain in dB
                   int channels,         // number of channels
                   double sampleRate)    // sample rate in Hz
    {
        assert(attackMs > 0.0f);
        assert(releaseMs > 0.0f);
        assert(channels > 0);
        assert(sampleRate > 0.0);

        mAttackInSamples = static_cast<size_t>(std::ceil(attackMs * static_cast<float>(sampleRate) / 1000.0f));
        mThreshold = std::pow(10.0f, thresholdInDb / 20.0f);
        mMakeupGain = std::pow(10.0f, makeupGainInDb / 20.0f);
        mChannels = channels;

        mAttackConst = std::pow(0.1f, 1.0f / (attackMs * static_cast<float>(sampleRate) / 1000.0f + 1.0f));
        mReleaseConst = std::pow(0.1f, 1.0f / (releaseMs * static_cast<float>(sampleRate) / 1000.0f + 1.0f));

        mMaxBuffer.resize(mAttackInSamples);
        mDelayBuffer.resize(mAttackInSamples * static_cast<size_t>(mChannels));

        reset();
    }

    // process audio data
    void process(float* buffer, int samples)
    {
        assert(mAttackInSamples > 0); // configure must be called beforehand

        for (int sample = 0; sample < samples; ++sample)
        {
            // store the maximum absolute sample value of all channels if the absolute value is greater than mThreshold
            mMaxBuffer[mBufferIndex] = mThreshold;
            for (int channel = 0; channel < mChannels; ++channel)
                mMaxBuffer[mBufferIndex] = std::max(mMaxBuffer[mBufferIndex], std::fabs(buffer[sample * mChannels + channel]));

            // get the maximum absolute sample value within mMaxBuffer
            if (mMaxBufferCurrentMaxIndex == mBufferIndex)
            {
                // there hasn't been a maximum or the last maximum just left the buffer
                const auto maxElementIterator = std::max_element(mMaxBuffer.begin(), mMaxBuffer.end());
                mMaxBufferCurrentMaxValue = *maxElementIterator;
                mMaxBufferCurrentMaxIndex = static_cast<size_t>(std::distance(mMaxBuffer.begin(), maxElementIterator));
            }
            else
            {
                // there is a valid maximum, we just need to check if the new value is bigger than the current maximum
                if (mMaxBuffer[mBufferIndex] >= mMaxBufferCurrentMaxValue)
                {
                    mMaxBufferCurrentMaxValue = mMaxBuffer[mBufferIndex];
                    mMaxBufferCurrentMaxIndex = mBufferIndex;
                }
            }

            const float currentGain = std::min(1.0f, mThreshold / mMaxBufferCurrentMaxValue);

            // avoid overshoot
            if (currentGain < mSmoothedState)
                mFadedGain = std::min(mFadedGain, (currentGain - 0.1f * mSmoothedState) * 1.11111111f);
            else
                mFadedGain = currentGain;

            if (mFadedGain < mSmoothedState)
            {
                // attack phase
                mSmoothedState = std::max(currentGain, mAttackConst * (mSmoothedState - mFadedGain) + mFadedGain);
            }
            else
            {
                // release phase
                mSmoothedState = mReleaseConst * (mSmoothedState - mFadedGain) + mFadedGain;
            }

            for (int channel = 0; channel < mChannels; ++channel)
            {
                // fill delay buffer
                const size_t currentDelayBufferIndex = mBufferIndex * static_cast<size_t>(mChannels) + static_cast<size_t>(channel);
                float sampleValue = mDelayBuffer[currentDelayBufferIndex];
                mDelayBuffer[currentDelayBufferIndex] = buffer[sample * mChannels + channel];

                // apply gain reduction
                sampleValue *= mSmoothedState;
                sampleValue = std::clamp(sampleValue, -mThreshold, mThreshold);

                // apply makeup gain
                sampleValue *= mMakeupGain;

                buffer[sample * mChannels + channel] = sampleValue;
            }

            ++mBufferIndex;
            if (mBufferIndex >= mAttackInSamples)
                mBufferIndex = 0;
        }
    }

    // resets the internal state
    void reset() noexcept
    {
        mBufferIndex = 0;

        mMaxBuffer.clear();
        mMaxBufferCurrentMaxValue = 0.0f;
        mMaxBufferCurrentMaxIndex = 0;

        mDelayBuffer.clear();

        mFadedGain = 1.0f;
        mSmoothedState = 1.0f;
    }

private:
    size_t mAttackInSamples = 0;
    float mAttackConst = 0.0f;
    float mReleaseConst = 0.0f;
    float mThreshold = 0.0f;
    float mMakeupGain = 0.0f;
    int mChannels = 0;

    size_t mBufferIndex = 0;

    std::vector<float> mMaxBuffer;
    float mMaxBufferCurrentMaxValue = 0.0f;
    size_t mMaxBufferCurrentMaxIndex = 0;

    std::vector<float> mDelayBuffer;

    float mFadedGain = 1.0f;
    float mSmoothedState = 1.0f;
};

} // namespace edsp
