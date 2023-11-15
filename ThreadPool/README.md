# ThreadPool
This thread pool implementation uses https://github.com/max0x7ba/atomic_queue.

The advantage of this implementation is that tasks can be submitted without triggering an allocation (e.g. from an audio thread). The size of the queue (tasks waiting for execution) must be specified at compile time.

## Usage

``` cpp
#include "ThreadPool/ThreadPool.h"

int maxQueueSize = 42;

// call e.g. at application start
edsp::ThreadPool<maxQueueSize> threadPool;

// call from audio thread
threadPool.enqueue(&myFunction, this, myFunctionParameter1, myFunctionParameter2);
```
