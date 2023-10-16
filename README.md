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
<img src="docs/Leopard.jpg" alt="ThreadPool Leopard" width="400" longdesc="https://htmlpreview.github.io/?https://github.com/hosseinmoein/ThreadPool/blob/master/README.md"/>

This is a light-weight C++ Thread Pool that allows running any callable similar to `std::async` interface. It has both global and local queues and employs a work-stealing algorithm.<BR>
It has the following interface:<BR>
1. You can run any function with any signature including member functions.
2. The thread pool is constructed with:
   1. Number of initial threads -- default is number of cores in your computer.
   2. Boolean flag to specify if there is a timeout for threads -- default is _false_. With timeout, if threads idle more than the timeout period, they will terminate.
   3. time_t value to specify the timeout period in seconds -- defaulted to 30 minutes. This is considered only if `timeout_flag` is _true_.
3. You `dispatch()` a routine. If a thread is available the routine will run. If no thread is available, the routine will be added to a queue until a thread becomes available. Dispatch interface is identical to `std::async()` with one exception:
   1. The first parameter is a Boolean flag named `immediately`. If true and no thread is available, a thread will be added to the pool and your routine runs immediately.
   2. The second parameter is a callable reference.
   3. The next parameter(s) are a variadic list of parameters matching your callable parameter list.
   4. `dispatch()` returns a `std::future` of the type of your callable return type.
4. There is also `parallel_loop()` method. This parallelizes a big class of problems very conveniently. It divides the data elements between begin and end to _n_ blocks by dividing by number of capacity threads. It dispatches the _n_ tasks.
   1. The first and second parameters are a pair of iterators, begin and end. They can either be proper iterators or integral type indices.
   2. The third parameter is a callable.
   3. There is also a variadic list of parameters at the end that must match the callable's parameter list.
   4. `parallel_loop()` returns a `std::vector` of `std::future` corresponding to the above _n_ tasks.
5. You can call `run_task()`. If the pool is not shutdown and there is a pending task in the queue, it runs it on the calling thread synchronously. It returns _true_, if a task was executed, otherwise _false_. The return value of the task could be obtained from the original _future_ object when it was dispatched. **NOTE**: A _false_ return from `run_task()` does not necessarily mean there were no tasks in thread pool queue. It might be that `run_task()` just encountered one of the thread pool internal maintenance tasks which it ignored and returned _false_.
6. At any point you can add/subtract threads to/from the pool by calling `add_thread(()`.
7. At any point you can query the ThreadPool for available or capacity threads, by calling `available_threads()` or `capacity_threads()`.
8. At any point you can query the ThreadPool for number of tasks currently waiting in the queue by calling `pending_tasks()`.
9. At any point you can call `shutdown()` to signal the ThreadPool to terminate all threads after they are done running routines. After shutdown, you cannot dispatch or add threads anymore -- exception will be thrown.
10. The destructor calls `shutdown()` and waits until all threads are done running routines.

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
    // Firs we do it using iterators
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
