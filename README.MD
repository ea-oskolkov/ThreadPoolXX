ThreadPoolXX
========================

# Overview
Library providing a thread pool.

# Requirements

* Compiler with С++20 support
* CMake

# Build and install

In the source code directory run:

* `mkdir -p build && pushd build`

* for static lib: `cmake -S ../`

* for shared lib: `cmake -S ../ -DBUILD_SHARED_LIBS:BOOL=true`

* to build the example, add ` -DBUILD_EXAMPLES:BOOL=true`

* `cmake --build .`

* `sudo cmake --install . --prefix /usr`

* `popd`

# Example of use

```C
    static constexpr size_t max_threads = 16;
    static constexpr size_t increment = 2;
    static constexpr size_t queue_capacity = 8;
    ThreadPoolXX thread_pool(max_threads, increment, queue_capacity);
    thread_pool.task_enqueue([&]{do_work();});
```

# LICENSE

This software is distributed under [MIT](https://opensource.org/licenses/MIT) license.
