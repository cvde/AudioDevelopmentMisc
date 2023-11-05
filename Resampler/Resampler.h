#pragma once

#include <memory>

class Resampler
{
public:
    Resampler();
    ~Resampler();
    Resampler(const Resampler&) = delete;
    Resampler& operator=(const Resampler&) = delete;

    void process(float* inputBuffer, float* outputBuffer, int numInputSamples, int numOutputSamples);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
