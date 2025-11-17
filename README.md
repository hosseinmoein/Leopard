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
[![GitHub tag (latest by date)](https://img.shields.io/github/tag-date/hosseinmoein/Leopard.svg?color=blue&label=Official%20Release&style=popout)](https://github.com/hosseinmoein/Leopard/releases)
[![Conan Center](https://img.shields.io/conan/v/leopard)](https://conan.io/center/recipes/leopard)

<img src="docs/Leopard.jpg" alt="ThreadPool Leopard" width="400" longdesc="https://htmlpreview.github.io/?https://github.com/hosseinmoein/ThreadPool/blob/master/README.md"/>

This is a light-weight C++ Thread Pool that allows running any callable similar to `std::async` interface. It has both global and local queues and employs a work-stealing algorithm.<BR>

## <a href="https://hosseinmoein.github.io/Leopard/docs/HTML/Leopard.html" target="_blank"><B>API Reference with code samples</B></a>


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
