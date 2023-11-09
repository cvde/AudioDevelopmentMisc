# ThreadPool
This thread pool implementation uses https://github.com/max0x7ba/atomic_queue.

The advantage of this implementation is that tasks can be submitted without triggering an allocation (e.g. from an audio thread). The size of the queue (tasks waiting for execution) must be specified at compile time.

## Usage

``` cpp
constexpr int MAX_QUEUE_SIZE = 42;
edsp::ThreadPool<MAX_QUEUE_SIZE> threadPool;

threadPool.enqueue(&myFunction, this, myFunctionParameter1, myFunctionParameter2);
```
