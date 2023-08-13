#include "thread_pool.hpp"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    using namespace std::literals::chrono_literals;

    ThreadPool pool(2);

    pool.enqueue_task([] {
        std::this_thread::sleep_for(100ms);
        std::cout << "Hey mom!\n";
    });

    const int a = 2;
    auto second_task = pool.enqueue_task([](const int& a) {
        std::this_thread::sleep_for(50ms);
        return 12 * a;
    }, a);

    std::cout << second_task.get() << std::endl;
}
