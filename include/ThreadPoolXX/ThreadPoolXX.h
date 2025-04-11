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
 * */
class ThreadPoolXX
{
public:
	ThreadPoolXX(const ThreadPoolXX&) = delete;
	ThreadPoolXX& operator=(const ThreadPoolXX&) = delete;

	using Task = std::function<void()>;

	/**
	 * @brief creates a thread pool with incremental growth of the pool.
	 * @param max_threads     maximum number of threads
	 * @param increment       increment when filling
	 * @param queue_capacity  task queue capacity
	 * @note First allocates @increment threads. When all @increment
	 * threads are exhausted, it will allocate @increment more threads.
	 * The number of threads is limited to @max_threads.
	 * */
	explicit ThreadPoolXX(
		size_t max_threads,
		size_t increment,
		size_t queue_capacity = std::numeric_limits<size_t>::max()
	);

	/**
	 * @note Blocks until all threads have stopped.
	 * */
	virtual ~ThreadPoolXX();

	/**
	 * @brief Adds a task to the queue for execution.
	 * If necessary, allocates additional threads.
	 * @param task   function executed in a thread.
	 * @note  This is a asynchronous method.
	 * @retval true   task added
	 * @retval false  the task queue is full or pool is stopped
	 * */
	bool enqueue(Task && task);

	/**
	 * @brief return number of running tasks.
	 * */
	std::size_t num_running_tasks() const noexcept
	{ return m_running_tasks_counter.load(std::memory_order_relaxed); }

	/**
	 * @brief return number of allocated threads.
	 * */
	std::size_t num_allocated_threads() const noexcept
	{ return m_running_threads_counter.load(std::memory_order_relaxed); }

private:
	const size_t m_conf_max_threads;
	const size_t m_conf_increment_threads;
	const size_t m_conf_queue_capacity;

	std::atomic_bool m_stopped{false};

	std::atomic_size_t m_running_tasks_counter{0};
	std::atomic_size_t m_running_threads_counter{0};

	std::vector<std::thread> m_threads{};

	mutable std::mutex m_lock{};
	std::mutex m_lock_done{};
	std::condition_variable m_notify_changes_state{};
	std::condition_variable m_notify_done{};

	std::queue<Task> m_queue_tasks{};

	bool P_should_be_expand_pool(
		size_t current_num_threads,
		size_t added_num_task
	) const noexcept;
	void P_expand_pool(size_t num);
	void P_thread();
};

#endif // THREADPOOLXX_THREADPOOLXX_H_
