#include <ThreadPoolXX/ThreadPoolXX.h>

#include <chrono>
#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] char ** argv)
{
	ThreadPoolXX thread_pool(16, 2);

	thread_pool.enqueue([&]{
		thread_pool.enqueue([]{
			for (size_t i = 0; i < 5; ++i)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << "2: Hello, World!" << std::endl;
			}
		});

		for (size_t i = 0; i < 5; ++i)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "1: Hello, World!" << std::endl;
		}
	});

	while (thread_pool.num_running_tasks() != 8);

	std::cout
		<< "Number of allocated threads:"
		<< thread_pool.num_allocated_threads()
		<< std::endl
		<< "Number of running tasks:"
		<< thread_pool.num_running_tasks()
		<< std::endl
		<< std::endl
	;

	return 0;
}
