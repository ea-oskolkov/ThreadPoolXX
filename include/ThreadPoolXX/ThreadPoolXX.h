#ifndef THREADPOOLXX_THREADPOOLXX_H_
#define THREADPOOLXX_THREADPOOLXX_H_

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>

/**
 * @class ThreadPoolXX
 *
 * @brief Thread pool implementation.
 * */
class ThreadPoolXX
{
public:
	using WorkerFunc = std::function<void()>;

	/**
	 * @brief incremental allocate.
	 * @param max_num     maximum number of threads
	 * @param increment   increment when filling
	 * @note First allocates @increment threads. When all @increment
	 * threads are exhausted, it will allocate @increment more threads.
	 * The number of threads is limited to @max_num.
	 * */
	explicit ThreadPoolXX(size_t max_num, size_t increment);

	/**
	 * @brief Allocate everything at once.
	 * @param max_num    maximum number of threads.
	 * */
	explicit ThreadPoolXX(size_t max_num)
	:ThreadPoolXX(max_num, max_num){};

	ThreadPoolXX(ThreadPoolXX&&) = delete;
	ThreadPoolXX(const ThreadPoolXX&) = delete;

	/**
	 * @note blocks until all threads have stopped.
	 * */
	virtual ~ThreadPoolXX();

	ThreadPoolXX& operator=(const ThreadPoolXX&) = delete;
	ThreadPoolXX& operator=(ThreadPoolXX&&) = delete;

	/**
	 * @brief Add task to queue.
	 * @param worker_func   function executed in a thread.
	 * @note  This is a synchronous method.
	 * */
	void enqueue(const WorkerFunc &worker_func);

	/**
	 * @brief return number of executed tasks.
	 * */
	std::size_t countExecutedTask() const noexcept
	{
		return m_count_executed_task;
	}

	/**
	 * @brief return number of allocated threads.
	 * */
	std::size_t size() const
	{
		std::lock_guard<std::mutex> lock(m_lock_workers);
		return m_workers.size();
	}

private:
	volatile bool m_run = true;

	const size_t m_conf_max_num;
	const size_t m_conf_increment;
	std::atomic_size_t m_count_executed_task{0};

	std::vector<std::thread> m_workers{};

	std::mutex m_lock{};
	mutable std::mutex m_lock_workers{};
	std::condition_variable m_cv{};

	std::queue<WorkerFunc> m_queue_tasks{};

	void P_worker();
	void P_extend_pool(size_t num);
};

#endif // THREADPOOLXX_THREADPOOLXX_H_
