#ifndef THREADPOOLXX_THREADPOOLXX_H
#define THREADPOOLXX_THREADPOOLXX_H

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
	using ThreadFunc = std::function<void()>;

	/**
	 * @brief incremental allocate.
	 * @param maxCount     maximum number of threads
	 * @param increment    increment when filling
	 * @note First allocates @increment threads. When all @increment
	 * threads are exhausted, it will allocate @increment more threads.
	 * The number of threads is limited to @maxCount.
	 * */
	explicit ThreadPoolXX(size_t maxCount, size_t increment);

	/**
	 * @brief Allocate everything at once.
	 * @param maxCount    maximum number of threads.
	 * */
	explicit ThreadPoolXX(size_t maxCount) : ThreadPoolXX(maxCount, maxCount){};

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
	 * @param threadFunc    function executed in a thread.
	 * @note  This is a synchronous method.
	 * */
	void enqueue(const ThreadFunc &threadFunc);

	/**
	 * @brief return number of executed tasks.
	 * */
	std::size_t countTask() const noexcept;

	/**
	 * @brief return number of allocated threads.
	 * */
	std::size_t size() const;

private:
	bool m_run;
	size_t m_maxCount;
	size_t m_increment;
	std::atomic_size_t m_countRunTask;

	std::vector<std::thread> m_workers{};

	std::mutex m_mutex{};
	mutable std::mutex m_workers_mutex{};
	std::condition_variable m_cv{};

	std::queue<ThreadFunc> m_tasks{};

	void worker();
	void extendPool(size_t count);
};


#endif
