#include <ThreadPoolXX/ThreadPoolXX.h>

#include <chrono>
#include <iostream>

using EnqueueResult = ThreadPoolXX::EnqueueResult;

static constexpr auto sleep_time = std::chrono::seconds(3);

static void P_print_enq_result(EnqueueResult enq_res)
{
#define PREFIX "result: "
    switch (enq_res)
    {
        case EnqueueResult::ERR__QUEUE_IS_FULL:
        {
            printf(PREFIX "ERR: QUEUE IS FULL\n");
            break;
        }
        case EnqueueResult::ERR__THREADS_STOPPED:
        {
            printf(PREFIX "ERR: THREADS STOPPED\n");
            break;
        }
        case EnqueueResult::OK:
        {
            printf(PREFIX "OK\n");
            break;
        }
    }
}

static void P_print_metric(const ThreadPoolXX & thread_pool)
{
    printf(
        "metrics: running tasks %zu, number of tasks queued: %zu,"
        "running threads: %zu, allocated threads: %zu\n",
        thread_pool.num_running_tasks(),
        thread_pool.num_tasks_queued(),
        thread_pool.num_running_threads(),
        thread_pool.num_allocated_threads()
    );
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char ** argv)
{
    static constexpr size_t max_threads = 16;
    static constexpr size_t increment = 2;
    static constexpr size_t queue_capacity = 2;
    ThreadPoolXX thread_pool(max_threads, increment, queue_capacity);

    for (size_t i = 0; i < (max_threads * 2); ++i)
    {
        static constexpr char lines_separator[] = "\n";

        printf("enqueue task: %zu\n", (i + 1));
        while (true)
        {
            const EnqueueResult enq_res = thread_pool.task_enqueue(
                []{ std::this_thread::sleep_for(sleep_time); }
            );
            P_print_enq_result(enq_res);

            if (enq_res == EnqueueResult::OK)
            {
                break;
            }

            printf("I'll try again!\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        P_print_metric(thread_pool);
        printf(lines_separator);
    }
    printf("Waiting for tasks to complete\n");
    return 0;
}
