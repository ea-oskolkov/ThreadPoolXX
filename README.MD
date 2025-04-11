ThreadPoolXX
========================

# Overview
Library providing a thread pool.

Mode support:
* allocate everything at once;
* incremental allocate.

# Requirements

* Compiler with С++14 support
* CMake

# Build

In the source code directory run:

* `mkdir -p build && pushd build`

* for static lib: `cmake -S ../`

* for shared lib: `cmake -S ../ -BUILD_SHARED_LIBS:BOOL=true`

* `cmake --build .`

* `popd`

# Usage:

```C
    ThreadPool thread_pool(10, 2);
    thread_pool.enqueue([&]{do_work();});
```

# LICENSE

This software is distributed under [MIT](https://opensource.org/licenses/MIT) license.
