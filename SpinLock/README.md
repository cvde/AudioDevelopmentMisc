# SpinLock
This spin lock loops until it gets the resource. It should only be used for very short waiting times.

The implementation is an adaptation of https://timur.audio/using-locks-in-real-time-audio-processing-safely with support for ARM processors.

⚠️ Completely untested on Windows / Visual Studio!

## Usage

``` cpp
#include "SpinLock/SpinLock.h"

edsp::SpinLock spinLock;

std::lock_guard<edsp::SpinLock> lock1(spinLock); // waits for the lock

std::unique_lock<edsp::SpinLock> lock2{spinLock, std::try_to_lock}; // does not wait for the lock
if (!lock2.owns_lock())
{
    // failed to acquire lock
}
```
