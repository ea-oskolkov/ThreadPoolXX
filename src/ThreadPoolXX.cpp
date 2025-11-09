#include <ThreadPoolXX/ThreadPoolXX.h>

#include <stdexcept>
#include <algorithm>
#include <stddef.h>

static size_t P_calc_expand_num(
    size_t amount_threads,
    size_t tasks_amount,
    size_t max_threads,
    size_t increment
) noexcept
{
    const bool free_threads = (amount_threads != tasks_amount);
    if (free_threads)
    {
        return 0;
    }
    const size_t free_quantity_in_pool = (max_threads - amount_threads);
    const size_t res = std::min(free_quantity_in_pool, increment);
    return res;
}

ThreadPoolXX::ThreadPoolXX(
    size_t max_threads,
    size_t increment,
    size_t queue_capacity,
    ExceptionHandler && exception_handler
)
: m_conf_max_threads(max_threads)
, m_conf_increment_threads(increment)
, m_conf_queue_capacity(queue_capacity)
, m_conf_exception_handler(std::move(exception_handler))
{
    if (increment == 0)
    {
        throw std::out_of_range("The increment size must be greater than 0");
    }

    m_threads.reserve(max_threads);

    const size_t expand_num = std::min(increment, max_threads);
    P_expand_pool(expand_num);
}

ThreadPoolXX::~ThreadPoolXX()
{
    P_stop();
    P_threads_waiting_complete();
    P_threads_join();
}

ThreadPoolXX::EnqueueResult ThreadPoolXX::task_enqueue(Task && task)
{
    {
        std::lock_guard lock(m_lock);
        if (m_stopped)
        {
            return EnqueueResult::ERR__THREADS_STOPPED;
        }

        const size_t tasks_in_queue = m_queue_tasks.size();
        if (tasks_in_queue >= m_conf_queue_capacity)
        {
            return EnqueueResult::ERR__QUEUE_IS_FULL;
        }

        const size_t tasks_amount = (tasks_in_queue + m_running_tasks_counter);
        const size_t expand_num = P_calc_expand_num(
            m_threads.size(),
            tasks_amount,
            m_conf_max_threads,
            m_conf_increment_threads
        );
        P_expand_pool(expand_num);

        m_queue_tasks.emplace(std::move(task));
    }

    m_notify_changes_state.notify_one();
    return EnqueueResult::OK;
}

void ThreadPoolXX::P_expand_pool(size_t num)
{
    for (size_t i = 0; i < num; ++i)
    {
        m_threads.emplace_back(&ThreadPoolXX::P_thread, this);
    }
}

void ThreadPoolXX::P_thread()
{
    ++m_running_threads_counter;

    while (!m_stopped)
    {
        Task task;
        {
            std::unique_lock lock(m_lock);
            m_notify_changes_state.wait(lock, [&]
            {
                const bool tasks_present = (!m_queue_tasks.empty());
                return (tasks_present || m_stopped);
            });

            if (m_stopped)
            {
                break;
            }

            task = std::move(m_queue_tasks.front());
            m_queue_tasks.pop();

            ++m_running_tasks_counter;
        }

        try
        {
            task();
        }
        catch (const std::exception & e)
        {
            if (m_conf_exception_handler != nullptr)
            { m_conf_exception_handler(&e); }
        }
        catch (...)
        {
            if (m_conf_exception_handler != nullptr)
            { m_conf_exception_handler(nullptr); }
        }
        --m_running_tasks_counter;
    }

    bool all_threads_done;
    {
        /**
         * Quote from https://en.cppreference.com/w/cpp/thread/condition_variable.html
         * "Even if the shared variable is atomic, it must be modified while
         * owning the mutex to correctly publish the modification to the waiting
         * thread."
         */
        std::lock_guard lock_done(m_lock_done);
        const size_t running_threads_num = --m_running_threads_counter;
        all_threads_done = (running_threads_num == 0);
    }

    if (all_threads_done)
    {
        m_notify_done.notify_one();
    }
}

void ThreadPoolXX::P_stop()
{
    {
        std::lock_guard lock(m_lock);
        m_stopped = true;
    }
    m_notify_changes_state.notify_all();
}

void ThreadPoolXX::P_threads_waiting_complete()
{
    std::unique_lock lock_done(m_lock_done);
    m_notify_done.wait(lock_done, [&]
    {
        return (m_running_threads_counter == 0);
    });
}

void ThreadPoolXX::P_threads_join()
{
    for (std::thread & thread : m_threads)
    {
        thread.join();
    }
}
