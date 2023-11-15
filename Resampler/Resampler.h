#pragma once

#include "../Debug/Debug.h"
#include <cassert>
#include <samplerate.h>

namespace edsp
{

class Resampler
{
public:
    Resampler(int channels)
    {
        assert(channels > 0);

        int error = 0;
        mState = src_new(SRC_SINC_FASTEST, channels, &error);

        if (mState == nullptr)
            DBG("src_new failed with error " << src_strerror(error));

        src_reset(mState);
    }
    ~Resampler()
    {
        if (mState != nullptr)
            src_delete(mState);
    }
    Resampler(const Resampler&) = delete;
    Resampler& operator=(const Resampler&) = delete;

    void process(float* inputBuffer, float* outputBuffer, int inputSamples, int outputSamples)
    {
        assert(inputSamples > 0);
        assert(outputSamples > 0);

        if (mState == nullptr)
            return;

        SRC_DATA data;
        data.data_in = inputBuffer;
        data.data_out = outputBuffer;
        data.input_frames = inputSamples;
        data.output_frames = outputSamples;
        data.end_of_input = 0;
        data.src_ratio = outputSamples / static_cast<double>(inputSamples);

        int error = 0;
        error = src_process(mState, &data);
        if (error != 0)
            DBG("src_process failed with error " << src_strerror(error));
    }

private:
    SRC_STATE* mState = nullptr;
};

} // namespace edsp
