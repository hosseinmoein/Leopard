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

This is a light-weight C++ Thread Pool that allows object member functions to run on threads.<BR>
It has the following features:<BR>
1. You can run any function with any signature including member functions.
2. The thread pool is constructed with:
   1. Number of initial threads
   2. Boolean flag to specify if there is a timeout for threads -- defaulted to true. With timeout, if threads idle more than the timeout period, they will terminate.
   3. time_t value to specify the timeout period in seconds -- defaulted to 30 minutes
3. You `dispatch()` a routine. If a thread is available the routine will run. If no thread is available, the routine will be added to a queue until a thread becomes available. Dispatch interface is identical to `std::async()` with one exception:
   1. The first parameter is a Boolean flag named `immediately`. If true and no thread is available, a thread will be added to the pool and your routine runs immediate.
   2. The second parameter is a callable reference.
   3. The next parameter(s) is a variadic list of parameters matching your callable parameter list.
   4. `dispatch()` returns a `std::future` of the type of your callable return type.
4. At any point you can add/subtract threads to/from the pool.
5. At any point you can query the ThreadPool for available or capacity threads, by calling `available_threads()` or `capacity_threads()`.
6. At any point you can call `shutdown()` to signal the ThreadPool to terminate all threads after they are done running routines. After shutdown, you cannot dispatch or add threads anymore -- exception will be thrown.
7. The destructor calls `shutdown()` and waits until all threads are done running routines.

```cpp
struct   MyClass  {
    bool routine(std::size_t i)  {
            struct timespec rqt;

            rqt.tv_sec = 1;
            rqt.tv_nsec = 0;
            ::nanosleep(&rqt, nullptr);

            std::cout << "From routine() " << i << "\n";
            return (true);
        }
};

// ----------------------------------------------------------------------------

std::string
my_func(std::size_t i, double d, const std::string &s)  {
    struct timespec rqt;

    rqt.tv_sec = 1;
    rqt.tv_nsec = 0;
    ::nanosleep(&rqt, nullptr);

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
    auto    my_func_fut = thr_pool.dispatch(false, my_func, 5555, 0.555, std::cref(str));
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
