# LookAheadLimiter
This implementation is based on https://github.com/tcarpent/PeakLimiter.

The following example limits the signal to -3 dBFS and then adds 2 dB (the output peaks at -1 dBFS).

## Usage

``` cpp
// call e.g. on application start
edsp::LookAheadLimiter limiter;
limiter.configure(10.0f, 50.0f, -3.0f, 2.0f, channels, sampleRate);

// call from audio thread
limiter.process(buffer, samples);
```
