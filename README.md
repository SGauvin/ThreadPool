# ThreadPool
Simple C++ thread pool.

# Requirements
A C++20 compiler. If you have a C++23 compiler and a library that supports it, `std::move_only_function` will be used instead of `std::function` to avoid an additional memory allocation.

# Usage
```cpp
static constexpr std::size_t num_threads = 4;
ThreadPool pool(num_threads);

using namespace std::literals::chrono_literals;

pool.enqueue_task([] {
    std::this_thread_sleep_for(100ms);
    std::cout << "Hi mom!\n";
});

const int num = 2;
// When a task returns a value, you must use the returned future
auto future = pool.enqueue_task([] (const int& num) {
    std::this_thread_sleep_for(50ms);
   return 2 + num;
}, num);

std::cout << future.get() << std::endl;
```
The destructor of the thread pool will wait for every task to be completed.
