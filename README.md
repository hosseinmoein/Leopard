<!--
Copyright (c) 2023-2028, Hossein Moein
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of Hossein Moein and/or the ThreadPool nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Hossein Moein BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-->

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/the-standard )
[![Conan Center](https://img.shields.io/conan/v/leopard)](https://conan.io/center/recipes/leopard)

<img src="docs/Leopard.jpg" alt="ThreadPool Leopard" width="400" longdesc="https://htmlpreview.github.io/?https://github.com/hosseinmoein/ThreadPool/blob/master/README.md"/>

This is a light-weight C++ Thread Pool that allows running any callable similar to `std::async` interface. It has both global and local queues and employs a work-stealing algorithm.<BR>

## ThreadPool API Reference<BR>

| **Member Function**        | <div style="width:290px">**Description**</div>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         | <div style="width:375px">**Parameters**</div>                                                                                                                                                                                                                                                                                                                                                                       | **Code Sample** |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- |
| <I>Constructor</I>         | `Conditioner` struct represents (is a wrapper around) a single executable function. See `ThreadPool.h`. It is used in ThreadPool to specify user custom initializers and cleanups per thread.<BR><BR>ThreadPool has only one constructor with all default values.<BR>Conditioner(s) are a handy interface, if threads need to be initialized before doing anything. And/or they need a cleanup before exiting. For example, see Windows CoInitializeEx function in COM library. See `conditioner_test()` in `thrpool_tester.cc` file for code sample    | <B>thr_num</B>: Number of initial threads -- defaulted to number of cores in the computer.<BR><B>pre_conditioner:</B> A function that will execute at the start of each thread in the pool once<BR><B>post_conditioner</B>: A function that will execute at the end of each thread in the pool once                                                                                  | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L383 |
| <I>set_timeout()</I>       | It enables/disables timeouts for threads. If timeout is enabled and a thread idles more than timeout period, it will terminate                                                                                                                                                                                                                                                                                                                                                                                                                          | <B>timeout_flag</B>: Boolean flag to enable/disable timeout mechanism<BR><B>timeout_time</B>: time_t value to specify the timeout period in seconds -- defaulted to 30 minutes                                                                                                                                                                                                       | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L94 |
| <I>dispatch()</I>          | It dispatches a task into the thread-pool queue. If a thread is available the task will run. If no thread is available, the task will be added to a queue until a thread becomes available. Dispatch interface is identical to`std::async()` with one exception.<BR>`dispatch()` returns a `std::future` of the type of your callable return type                                                                                                                                                                                                       | <B>immediately</B>: A boolean flag, If true and no thread is available, a thread will be added to the pool and the task runs immediately.<BR><B>routine</B>: A callable reference<BR><B>args ...</B>: A variadic list of parameters matching your callable parameter list                                                                                                            | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L209 |
| <I>parallel_loop()</I>     | It parallelizes a big class of problems very conveniently. It divides the data elements between begin and end to _n_ blocks by dividing by number of capacity threads. It dispatches the _n_ tasks.<BR> `parallel_loop()` returns a `std::vector` of `std::future` corresponding to the above _n_ tasks.                                                                                                                                                                                                                                                | <B>begin</B>: An iterator to mark the beginning of the sequence. It can either be proper a iterator or integral type index<BR><B>end</B>: An iterator to mark the end of the sequence. It can either be proper a iterator or integral type index<BR><B>routine</B>: A reference to a callable<BR><B>args ...</B>: A variadic list of parameters matching your callable parameter list | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L272 |
| <I>parallel_sort()</I>     | It uses a parallel version of 3-way quick sort.<BR>This version calls the quick sort with std::less as comparison functor | <B>begin</B>: An iterator to mark the beginning of the sequence.<BR><B>end</B>: An iterator to mark the end of the sequence<BR><B>TH (template param)</B>:A threshold value below which a serialize sort will be used, defaulted to 500,000 | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L421 |
| <I>parallel_sort()</I>     | It uses a parallel version of 3-way quick sort.<BR>This version requires a comparison functor | <B>begin</B>: An iterator to mark the beginning of the sequence.<BR><B>end</B>: An iterator to mark the end of the sequence<BR><B>compare</B>: Comparison functor<BR><B>TH (template param)</B>:A threshold value below which a serialize sort will be used, defaulted to 500,000 | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L421 |
| <I>attach()</I>            | It attaches the current thread to the pool so that it may be used for executing submitted tasks. It blocks the calling thread until the pool is shutdown or the thread is timed-out. This is handy, if you already have thread(s), and want to repurpose them                                                                                                                                                                                                                                                                                           | <B>this_thr</B>: The parameter is a rvalue reference to the std::thread instance of the calling thread                                                                                                                                                                                                                                                                               | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L319 |
| <I>run_task()</I>          | If the pool is not shutdown and there is a pending task in the queue, it runs it on the calling thread synchronously. It returns_true_, if a task was executed, otherwise _false_. The return value of the task could be obtained from the original _future_ object when it was dispatched.<BR>**NOTE**: A _false_ return from `run_task()` does not necessarily mean there were no tasks in thread pool queue. It might be that `run_task()` just encountered one of the thread pool internal maintenance tasks which it ignored and returned _false_. |                                                                                                                                                                                                                                                                                                                                                                                      | https://github.com/hosseinmoein/Leopard/blob/main/test/par_sort_tester.cc#L51 |
| <I>add_thread()</I>        | At any point you can add/subtract threads to/from the pool                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              | <B>thr_num</B>: Number of threads to be added/subtracted. It can either be a positive or negative number                                                                                                                                                                                                                                                                             | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L94 |
| <I>available_threads()</I> | At any point you can query the ThreadPool for available threads.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |                                                                                                                                                                                                                                                                                                                                                                                      | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L94 |
| <I>capacity_threads()</I>  | At any point you can query the ThreadPool for capacity threads.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |                                                                                                                                                                                                                                                                                                                                                                                      | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L94 |
| <I>pending_tasks()</I>     | At any point you can query the ThreadPool for number of tasks currently waiting in the_global_ queue                                                                                                                                                                                                                                                                                                                                                                                                                                                    |                                                                                                                                                                                                                                                                                                                                                                                      | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L94 |
| <I>shutdown()</I>          | At any point you can call`shutdown()` to signal the ThreadPool to terminate all threads after they are done running tasks. After shutdown, you cannot dispatch or add threads anymore -- exception will be thrown.                                                                                                                                                                                                                                                                                                                                      |                                                                                                                                                                                                                                                                                                                                                                                      | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L232 |
| <I>is_shutdown()</I>       | It returns_true_ if the pool is shutdown, otherwise _false_                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |                                                                                                                                                                                                                                                                                                                                                                                      | https://github.com/hosseinmoein/Leopard/blob/main/test/thrpool_tester.cc#L94 |
| <I>Destructor</I>          | It calls`shutdown()` and waits until all threads are done running tasks                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |                                                                                                                                                                                                                                                                                                                                                                                      | N/A  |

## Code Sample<BR>

```cpp
static void parallel_loop_test()  {

    std::cout << "Running parallel_loop_test() ..." << std::endl;

    constexpr std::size_t       n { 10003 };
    constexpr std::size_t       the_sum { (n * (n + 1)) / 2 };
    std::vector<std::size_t>    vec (n);

    std::iota(vec.begin(), vec.end(), 1);

    auto        func =
        [](const auto &begin, const auto &end) -> std::size_t  {
            std::size_t sum { 0 };

            for (auto iter = begin; iter != end; ++iter)
                sum += *iter;
            return (sum);
        };
    ThreadPool  thr_pool { 5 };  // Thread pool with 5 threads.
    // First we do it using iterators
    //
    auto        futs = thr_pool.parallel_loop(vec.cbegin(), vec.cend(), func);

    std::size_t result {0};

    for (auto &fut : futs)
        result += fut.get();
    assert(result == the_sum);

    // Now do the same thing, this time with integer indices
    //
    auto    func2 =
        [](auto begin, auto end, const auto &vec) -> std::size_t  {
            std::size_t sum { 0 };

            for (auto i = begin; i != end; ++i)
                sum += vec[i];
            return (sum);
        };
    auto    futs2 = thr_pool.parallel_loop(std::size_t(0), n, func2, vec);

    result = 0;
    for (auto &fut : futs2)
        result += fut.get();
    assert(result == the_sum);
}

// ----------------------------------------------------------------------------

struct   MyClass  {
    bool routine(std::size_t i)  {
        std::cout << "From routine() " << i << "\n";
        return (true);
    }
};

// ----------------------------------------------------------------------------

std::string
my_func(std::size_t i, double d, const std::string &s)  {
    std::cout << "my_func(): " << i << ", " << d << ", " << s << '\n';
    return (s + "_ABC");
}

// ----------------------------------------------------------------------------

int main (int, char *[])  {
    MyClass my_obj;

    // Start off with 5 threads in reserve in the pool
    // "timeout_flag" is true by default
    // "timeout_time" is set to half an hour for idle threads by default
    //
    ThreadPoolType  thr_pool (5);

    thr_pool.add_thread (2);  // Add 2 threads to the pool
    thr_pool.add_thread (-3); // Terminate 3 threads in the pool

    // Dispatch a member function
    // "immediately" flag is set to false
    //
    auto    my_class_fut = thr_pool.dispatch(false, &MyClass::routine, &my_obj, 5555);
    std::cout << "MyClass Future result: " << my_class_fut.get() << std::endl;

    // Dispatch a lambda
    // "immediately" flag is set to true
    //
    auto    my_lambda_fut = thr_pool.dispatch(true,
                                              [](std::size_t i) -> std::size_t { return (i * i); },
                                              10);
    std::cout << "Lambda result: " << my_lambda_fut.get() << std::endl;

    // Dispatch a function
    // "immediately" flag is set to false
    //
    const std::string   str = "XXXX";
    auto                my_func_fut = thr_pool.dispatch(false, my_func, 5555, 0.555, std::cref(str));
    std::cout << "my_func Future result: " << my_func_fut.get() << std::endl;

    std::cout << "Available threads: " << thr_pool.available_threads() << std::endl;
    std::cout << "Capacity threads: " << thr_pool.capacity_threads() << std::endl;

    // You don't have to call shutdown() necessarily, but you could.
    // It is called by the destructor.
    //
    thr_pool.shutdown();
    return (EXIT_SUCCESS);
}
```
