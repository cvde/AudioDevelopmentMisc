#include "Resampler.h"
#include "samplerate.h"
#include <cassert>

constexpr int CHANNELS = 2;

struct Resampler::Impl
{
    SRC_STATE* mState = nullptr;
};

Resampler::Resampler()
{
    pImpl = std::make_unique<Impl>();

    int error = 0;
    pImpl->mState = src_new(SRC_SINC_FASTEST, CHANNELS, &error);

    if (pImpl->mState == nullptr)
    {
        // DBG("src_new failed with error " << src_strerror(error));
    }

    src_reset(pImpl->mState);
}

Resampler::~Resampler()
{
    if (pImpl->mState != nullptr)
        src_delete(pImpl->mState);
}

void Resampler::process(float* inputBuffer, float* outputBuffer, int numInputSamples, int numOutputSamples)
{
    assert(inputBuffer != nullptr);
    assert(outputBuffer != nullptr);
    assert(numInputSamples > 0);
    assert(numOutputSamples > 0);

    if (pImpl->mState == nullptr)
        return;

    SRC_DATA data;
    data.data_in = inputBuffer;
    data.data_out = outputBuffer;
    data.input_frames = numInputSamples;
    data.output_frames = numOutputSamples;
    data.end_of_input = 0;
    data.src_ratio = numOutputSamples / (double)numInputSamples;

    int error = 0;
    error = src_process(pImpl->mState, &data);
    if (error != 0)
    {
        // DBG("src_process failed with error " << src_strerror(error));
    }
}
