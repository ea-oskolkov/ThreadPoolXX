#include "ThreadPoolXX.h"

ThreadPoolXX::ThreadPoolXX(size_t maxCount, size_t increment):
m_run(true), m_maxCount(maxCount), m_increment(increment), m_countRunTask(0)
{
	if (increment == 0)
	{
		throw std::invalid_argument("must be greater than 0");
	}

	const size_t allocCount = std::min(increment, maxCount);

	m_workers.reserve(maxCount);
	extendPool(allocCount);
}

ThreadPoolXX::~ThreadPoolXX()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_run = false;
	}
	m_cv.notify_all();

	std::lock_guard<std::mutex> lock(m_workers_mutex);
	for (auto &worker : m_workers)
	{
		worker.join();
	}
}

void ThreadPoolXX::enqueue(const ThreadPoolXX::ThreadFunc &threadFunc)
{
	{
		std::lock_guard<std::mutex> lock(m_workers_mutex);
		const size_t count_threads = m_workers.size();
		if(
				(count_threads < m_maxCount) &&
				(m_countRunTask >= m_increment) &&
				(m_countRunTask % m_increment) == 0
		)
		{
			const size_t remainder = m_maxCount - count_threads;
			const size_t allocCount = std::min(remainder, m_increment);

			extendPool(allocCount);
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tasks.emplace(threadFunc);
	}

	m_cv.notify_one();
}

void ThreadPoolXX::worker()
{
	while (true)
	{
		ThreadFunc workFunc;

		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_cv.wait(lock, [&]{return !m_run || !m_tasks.empty();});

			if (!m_run)
			{
				// Stop thread
				break;
			}

			workFunc = std::move(m_tasks.front());
			m_tasks.pop();
		}

		++m_countRunTask;
		workFunc();
		--m_countRunTask;
	}
}

void ThreadPoolXX::extendPool(size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		m_workers.emplace_back(&ThreadPoolXX::worker, this);
	}
}

std::size_t ThreadPoolXX::countTask() const noexcept
{
	return m_countRunTask;
}

std::size_t ThreadPoolXX::size() const
{
	std::lock_guard<std::mutex> lock(m_workers_mutex);
	return m_workers.size();
}
