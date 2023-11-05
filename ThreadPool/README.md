# ThreadPool
This thread pool implementation uses https://github.com/max0x7ba/atomic_queue
This makes it possible to submit tasks without triggering an allocation (lock free). The downside is that the maximum number of tasks submitted in parallel must be specified in advance.

## Usage

``` cpp
constexpr int MAX_QUEUE_SIZE = 42;
ThreadPool<MAX_QUEUE_SIZE> threadPool;

threadPool.enqueue(&myFunction, this, myFunctionParameter1, myFunctionParameter2);
```
