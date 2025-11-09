#ifndef THREADPOOLXX_THREADPOOLXX_H_
#define THREADPOOLXX_THREADPOOLXX_H_

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>
#include <limits>
#include <stddef.h>

/**
 * @class ThreadPoolXX
 *
 * @brief Thread pool implementation.
 * @note MT-safe.
 * @note It is recommended to use objects of this class in RAII style.
 * */
class ThreadPoolXX
{
public:
    using Task = std::function<void()>;

    /**
     * @brief   Task exception handler.
     * @note    nullptr is passed to the handler if the exception is not
     *          inherited from std::exception.
     */
    using ExceptionHandler = std::function<void(const std::exception*)>;

    enum class EnqueueResult
    {
        ERR__THREADS_STOPPED,
        ERR__QUEUE_IS_FULL,
        OK,
    };

    ThreadPoolXX(const ThreadPoolXX&) = delete;
    ThreadPoolXX& operator=(const ThreadPoolXX&) = delete;

    ThreadPoolXX() = delete;

    /**
     * @brief creates a thread pool with incremental growth of the pool.
     * @param max_threads         maximum number of threads
     * @param increment           increment when filling
     * @param queue_capacity      task queue capacity
     * @param exception_handler   task exception handler
     * @note First allocates @increment threads. When all @increment
     * threads are exhausted, it will allocate @increment more threads.
     * The number of threads is limited to @max_threads.
     * */
    ThreadPoolXX(
        size_t max_threads,
        size_t increment,
        size_t queue_capacity,
        ExceptionHandler && exception_handler = {}
    );

    /**
     * @note Blocks until all threads have stopped.
     * @note Will wait for already running tasks to complete.
     * @note Queued but not started tasks will be discarded.
     * @note The task_enqueue() will return the false.
     * */
    virtual ~ThreadPoolXX();

    /**
     * @brief Adds a task to the queue for execution.
     * If necessary, allocates additional threads.
     * @param task   function executed in a thread.
     * @note   This is a asynchronous method.
     * @note   A task can queue another task.
     * */
    EnqueueResult task_enqueue(Task && task);

    /**
     * @brief Return number of running tasks.
     * */
    std::size_t num_running_tasks() const noexcept
    { return m_running_tasks_counter.load(); }

    /**
     * @brief Return number of tasks in the queue.
     * */
    std::size_t num_tasks_queued() const noexcept
    {
        std::lock_guard lock(m_lock);
        return m_queue_tasks.size();
    }

    /**
     * @brief Return number of running threads.
     * */
    std::size_t num_running_threads() const noexcept
    { return m_running_threads_counter.load(); }

    /**
     * @brief Return number of allocated threads.
     * */
    std::size_t num_allocated_threads() const noexcept
    {
        std::lock_guard lock(m_lock);
        return m_threads.size();
    }

    /**
     * @brief Thread pool state.
     * @retval true   stopped
     */
    bool stopped() const noexcept
    { return m_stopped.load(); }

private:
    const size_t m_conf_max_threads;
    const size_t m_conf_increment_threads;
    const size_t m_conf_queue_capacity;

    ExceptionHandler m_conf_exception_handler{};

    std::atomic_bool m_stopped{false};

    std::atomic_size_t m_running_tasks_counter{0};
    std::atomic_size_t m_running_threads_counter{0};

    std::vector<std::thread> m_threads{};

    mutable std::mutex m_lock{};
    std::mutex m_lock_done{};
    std::condition_variable m_notify_changes_state{};
    std::condition_variable m_notify_done{};

    std::queue<Task> m_queue_tasks{};

    void P_expand_pool(size_t num);
    void P_thread();
    void P_stop();
    void P_threads_waiting_complete();
    void P_threads_join();
};

#endif // THREADPOOLXX_THREADPOOLXX_H_
