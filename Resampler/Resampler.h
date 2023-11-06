#pragma once

#include <memory>

class Resampler
{
public:
    Resampler(int channels);
    ~Resampler();
    Resampler(const Resampler&) = delete;
    Resampler& operator=(const Resampler&) = delete;

    void process(float* inputBuffer, float* outputBuffer, int inputSamples, int outputSamples);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
