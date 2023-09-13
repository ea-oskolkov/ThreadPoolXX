#include <ThreadPoolXX/ThreadPoolXX.h>

#include <stdexcept>
#include <algorithm>

ThreadPoolXX::ThreadPoolXX(size_t max_num, size_t increment):
	m_conf_max_num(max_num), m_conf_increment(increment)
{
	if (increment == 0)
	{
		throw std::invalid_argument("Arg 'increment' must be greater than 0.");
	}

	const size_t alloc_num = std::min(increment, max_num);

	m_workers.reserve(max_num);
	P_extend_pool(alloc_num);
}

ThreadPoolXX::~ThreadPoolXX()
{
	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_run = false;
	}
	m_cv.notify_all();

	std::lock_guard<std::mutex> lock(m_lock_workers);
	for (auto &worker : m_workers)
	{
		worker.join();
	}
}

void ThreadPoolXX::enqueue(const ThreadPoolXX::WorkerFunc &worker_func)
{
	{
		std::lock_guard<std::mutex> lock(m_lock_workers);
		const size_t num_threads = m_workers.size();
		if(
			(num_threads < m_conf_max_num) &&
			(m_count_executed_task >= m_conf_increment) &&
			(m_count_executed_task % m_conf_increment) == 0
		)
		{
			const size_t remainder = m_conf_max_num - num_threads;
			const size_t alloc_num = std::min(remainder, m_conf_increment);

			P_extend_pool(alloc_num);
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_queue_tasks.emplace(worker_func);
	}
	m_cv.notify_one();
}

void ThreadPoolXX::P_worker()
{
	while (true)
	{
		WorkerFunc worker_func;

		{
			std::unique_lock<std::mutex> lock(m_lock);
			m_cv.wait(lock, [&]{return !m_run || !m_queue_tasks.empty();});

			if (!m_run)
			{
				// Stop thread
				break;
			}

			worker_func = std::move(m_queue_tasks.front());
			m_queue_tasks.pop();
		}

		++m_count_executed_task;
		worker_func();
		--m_count_executed_task;
	}
}

void ThreadPoolXX::P_extend_pool(size_t num)
{
	for (size_t i = 0; i < num; ++i)
	{
		m_workers.emplace_back(&ThreadPoolXX::P_worker, this);
	}
}
