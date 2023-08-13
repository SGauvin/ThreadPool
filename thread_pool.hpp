#include <concepts>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <version>

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t thread_count) noexcept
    {
        m_threads.reserve(thread_count);
        for (std::size_t i = 0; i < thread_count; i++)
        {
            m_threads.emplace_back([this] (std::stop_token stop_token) {
                while (true)
                {
                    std::unique_lock lock(m_mutex);
                    m_condition_variable.wait(lock, stop_token, [this, stop_token] {
                        return !m_tasks.empty();
                    });
                    
                    if (stop_token.stop_requested() && m_tasks.empty())
                    {
                        return;
                    }

                    auto task = std::move(m_tasks.front());
                    m_tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(const ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&&) = delete;

    template <typename F, typename... Args, typename U = std::invoke_result_t<F, Args...>>
    requires(std::invocable<F, Args...> && !std::is_void_v<U>)
    [[nodiscard]] std::future<U> enqueue_task(F function, Args&&... args) noexcept
    {
        return private_enqueue_task(function, std::forward<Args>(args)...);
    }

    template <typename F, typename... Args>
    requires(std::invocable<F, Args...> && std::is_void_v<std::invoke_result_t<F, Args...>>)
    std::future<void> enqueue_task(F function, Args&&... args) noexcept
    {
        return private_enqueue_task(function, std::forward<Args>(args)...);
    }

private:
    template <typename F, typename... Args, typename U = std::invoke_result_t<F, Args...>>
    std::future<U> private_enqueue_task(F function, Args&&... args) noexcept
    {
#if defined(__cpp_lib_move_only_function) // C++23's move-only functions
        std::packaged_task<std::invoke_result_t<F, Args...>()> task([&] {
            return function(std::forward<Args>(args)...);
        });
        auto future = task.get_future();

        {
            std::scoped_lock lock(m_mutex);
            m_tasks.push(std::move(task));
        }
#else
        // In C++20, we must use std::function. We can't move the packaged_task into the function
        // with lambda capture, since it would make the function non-copyable. We must use C++23's
        // std::move_only_function for this.
        auto task_pointer = new std::packaged_task<std::invoke_result_t<F, Args...>()>([&] {
            return function(std::forward<Args>(args)...);
        });
        auto future = task_pointer->get_future();

        {
            std::scoped_lock lock(m_mutex);
            m_tasks.emplace([task_pointer] {
                (*task_pointer)();
                delete task_pointer;
            });
        }
#endif

        m_condition_variable.notify_one();
        return future;
    }

    std::mutex m_mutex;
    std::condition_variable_any m_condition_variable;
    std::vector<std::jthread> m_threads;

#if defined(__cpp_lib_move_only_function) // C++23's move-only functions
    std::queue<std::move_only_function<void ()>> m_tasks;
#else
    std::queue<std::function<void ()>> m_tasks;
#endif
};
