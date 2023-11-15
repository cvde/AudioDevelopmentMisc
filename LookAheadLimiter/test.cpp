// SPDX-FileCopyrightText: 2023 Christian Voigt
// SPDX-License-Identifier: MIT

#include "../AudioBuffer/AudioBufferHelpers.h"
#include "../AudioBuffer/AudioBufferInterleaved.h"
#include "LookAheadLimiter.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

static void fillBufferWithSineWaveAndPeak(float* buffer, int channels, int samples)
{
    float angularFrequency = 2.0f * static_cast<float>(M_PI) * 440.0f;
    float peakAmplitude = std::pow(10.0f, -5.0f / 20.0f); // peaks at -5 dBFS

    for (int sample = 0; sample < samples; ++sample)
    {
        for (int channel = 0; channel < 2; ++channel)
        {
            buffer[sample * channels + channel] = peakAmplitude * std::sinf(angularFrequency * sample / samples);
            if (sample == samples / 2)
                buffer[sample * channels + channel] = 1.0f; // one additional peak at 0 dBFS
        }
    }
}

static void writeBufferToFile(const float* buffer, int channels, int samples, const std::string& filePath)
{
    std::ofstream outFile(filePath);
    if (!outFile.is_open())
        return;

    outFile << std::fixed << std::setprecision(5);
    for (int sample = 0; sample < samples; ++sample)
    {
        for (int channel = 0; channel < channels; ++channel)
        {
            outFile << buffer[sample * channels + channel] << ",";
        }
        outFile << "\n";
    }
    outFile.close();
}

static bool compareFiles(const std::string& file1, const std::string& file2)
{
    std::ifstream f1(file1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(file2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail())
        return false; // file problem

    if (f1.tellg() != f2.tellg())
        return false; // size mismatch

    // seek back to beginning and use std::equal to compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}

int main()
{
    constexpr int channels = 2;
    constexpr int samples = 2048;
    constexpr double sampleRate = 48000.0;
    std::string comparisonFile = "testOutput.txt";
    std::string outputFile = "testOutputNew.txt";

    edsp::AudioBufferInterleaved<float> buffer{channels, samples};
    auto bufferPtr = buffer.getWritePointer();

    edsp::clearBuffer(bufferPtr, channels, samples);
    fillBufferWithSineWaveAndPeak(bufferPtr, channels, samples / 2);

    edsp::LookAheadLimiter limiter;
    limiter.configure(10.0f, 50.0f, -3.0f, 2.0f, channels, sampleRate);
    limiter.process(bufferPtr, samples);

    writeBufferToFile(bufferPtr, channels, samples, outputFile);
    if (compareFiles(comparisonFile, outputFile))
        std::cout << "Output hasn't changed.\n";
    else
        std::cout << "Output has changed!\n";

    return 0;
}
