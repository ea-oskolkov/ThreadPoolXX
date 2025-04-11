#include <ThreadPoolXX/ThreadPoolXX.h>

#include <stdexcept>
#include <algorithm>
#include <stddef.h>

ThreadPoolXX::ThreadPoolXX(size_t max_threads, size_t increment, size_t queue_size):
	m_conf_max_threads(max_threads),
	m_conf_increment_threads(increment),
	m_conf_queue_capacity(queue_size)
{
	if (increment == 0)
	{
		throw std::invalid_argument("Arg 'increment' must be greater than 0.");
	}

	m_threads.reserve(max_threads);

	const size_t expand_num = std::min(increment, max_threads);
	P_expand_pool(expand_num);
}

ThreadPoolXX::~ThreadPoolXX()
{
	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_stopped.store(true, std::memory_order_relaxed);
	}
	m_notify_changes_state.notify_all();

	{
		std::unique_lock<std::mutex> lock_done(m_lock_done);
		m_notify_done.wait(lock_done, [&]{
			return (m_running_threads_counter.load(std::memory_order_acquire) == 0);
		});
	}

	for (std::thread & thread : m_threads)
	{
		thread.join();
	}
}

bool ThreadPoolXX::enqueue(Task && task)
{
	if (m_stopped.load(std::memory_order_relaxed))
	{
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(m_lock);

		const size_t tasks_in_queue = m_queue_tasks.size();
		if (tasks_in_queue >= m_conf_queue_capacity)
		{
			return false;
		}

		const size_t current_num_threads = m_threads.size();
		const size_t added_num_task = (tasks_in_queue + m_running_tasks_counter.load(std::memory_order_relaxed));
		const bool should_be_expanded = P_should_be_expand_pool(
			current_num_threads,
			added_num_task
		);
		if(should_be_expanded)
		{
			const size_t free_quantity = (m_conf_max_threads - current_num_threads);
			const size_t expand_num = std::min(free_quantity, m_conf_increment_threads);
			P_expand_pool(expand_num);
		}

		m_queue_tasks.emplace(std::move(task));
	}
	
	m_notify_changes_state.notify_one();
	return true;
}

bool ThreadPoolXX::P_should_be_expand_pool(
	size_t current_num_threads,
	size_t added_num_task
)const noexcept
{
	const bool result =
		(current_num_threads < m_conf_max_threads) &&
		(added_num_task >= m_conf_increment_threads) &&
		(added_num_task %  m_conf_increment_threads) == 0;

	return result;
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
	m_running_threads_counter.fetch_add(1, std::memory_order_release);

	while (!m_stopped.load(std::memory_order_relaxed))
	{
		Task task;

		{
			std::unique_lock<std::mutex> lock(m_lock);
			m_notify_changes_state.wait(lock, [&]
			{
				const bool added_task = (!m_queue_tasks.empty());
				return (added_task || m_stopped.load(std::memory_order_relaxed));
			});

			if (m_stopped.load(std::memory_order_relaxed))
			{
				break;
			}

			task = std::move(m_queue_tasks.front());
			m_queue_tasks.pop();

			m_running_tasks_counter.fetch_add(1, std::memory_order_relaxed);
		}

		try
		{
			task();
		}
		catch (...)
		{
			// Do nothing
		}
		m_running_tasks_counter.fetch_sub(1, std::memory_order_relaxed);
	}

	bool all_threads_done;

	{
		std::lock_guard<std::mutex> lock_done(m_lock_done);
		const size_t running_threads_num =
			(m_running_threads_counter.fetch_sub(1, std::memory_order_release) - 1);
		all_threads_done = (running_threads_num == 0);
	}

	if (all_threads_done)
	{
		m_notify_done.notify_one();
	}
}
