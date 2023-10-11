// Hossein Moein
// August 9, 2023
/*
Copyright (c) 2023-2028, Hossein Moein
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Hossein Moein and/or the Leopard nor the
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
*/

#include <Leopard/ThreadPool.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using namespace hmthrp;

// ----------------------------------------------------------------------------

static constexpr std::size_t    THREAD_COUNT = 5;
static std::mutex               GLOCK { };

// ----------------------------------------------------------------------------

struct  MyClass  {

    MyClass()  { std::cout << "MyClass is Constructed" << std::endl; }
    ~MyClass()  { std::cout << "MyClass is Destructed" << std::endl; }

    bool routine(std::size_t i)  {

            struct timespec rqt;

            rqt.tv_sec = 1;
            rqt.tv_nsec = 0;
            ::nanosleep(&rqt, nullptr);

            const std::lock_guard<std::mutex>   lock { GLOCK };

            std::cout << "From MyClass::routine(): " << i << std::endl;
            return (true);
        }
};

MyClass my_obj;

// ----------------------------------------------------------------------------

std::string
my_func(std::size_t i, double d, const std::string &s)  {

    struct timespec rqt;

    rqt.tv_sec = 1;
    rqt.tv_nsec = 0;
    ::nanosleep(&rqt, nullptr);

    const std::lock_guard<std::mutex>   lock { GLOCK };

    std::cout << "From my_func(): " << i << ", " << d << ", " << s << std::endl;
    return (s + "_ABC");
}

// ----------------------------------------------------------------------------

static void haphazard()  {

    std::cout << "Running haphazard() ..." << std::endl;

    ThreadPool  thr_pool (THREAD_COUNT, true, 10);

    thr_pool.add_thread(20);

    struct timespec rqt;

    rqt.tv_sec = 11;
    rqt.tv_nsec = 0;
    ::nanosleep (&rqt, nullptr);

    std::cout << "Initial capacity: " << thr_pool.capacity_threads()
              << " -- available: " << thr_pool.available_threads()
              << std::endl;

    thr_pool.add_thread(2);

    rqt.tv_sec = 2;
    rqt.tv_nsec = 0;
    ::nanosleep(&rqt, nullptr);  // Give it time to take effect

    std::cout << "After adding 2 threads capacity: "
              << thr_pool.capacity_threads()
              << " -- available: " << thr_pool.available_threads()
              << std::endl;

    thr_pool.add_thread(-2);
    ::nanosleep(&rqt, nullptr);  // Give it time to take effect

    std::cout << "After adding -2 threads capacity: "
              << thr_pool.capacity_threads()
              << " -- available: " << thr_pool.available_threads()
              << std::endl;

    for (std::size_t i = 0; i < THREAD_COUNT * 100; ++i)
        thr_pool.dispatch((! (i % 10)) ? true : false,
                          &MyClass::routine, &my_obj, i);

    {
        const std::lock_guard<std::mutex>   lock { GLOCK };

        std::cout << "First batch is done ..." << std::endl;
    }
    ::nanosleep (&rqt, nullptr);

    for (std::size_t i = 0; i < THREAD_COUNT * 100; ++i)
        thr_pool.dispatch(false, &MyClass::routine, &my_obj, i);

    auto    my_class_fut =
        thr_pool.dispatch(false, &MyClass::routine, &my_obj, 5555);

    std::cout << "MyClass Future result: " << my_class_fut.get()
              << std::endl;
    {
        const std::lock_guard<std::mutex>   lock { GLOCK };

        std::cout << "Second batch is done ..." << std::endl;
    }

    ::nanosleep (&rqt, nullptr);
    for (std::size_t i = 0; i < THREAD_COUNT * 100; ++i)
        thr_pool.dispatch(
            (! (i % 10)) ? true : false,
            [](std::size_t i) -> std::size_t {
                const std::lock_guard<std::mutex>   lock { GLOCK };

                std::cout << "From Lambda: " << i * i << std::endl;
                return (i * i);
            },
            i);

    {
        const std::lock_guard<std::mutex>   lock { GLOCK };

        std::cout << "Third batch is done ..." << std::endl;
        std::cout << "Number of pending tasks: " << thr_pool.pending_tasks()
                  << std::endl;
    }
    rqt.tv_sec = 10;
    ::nanosleep (&rqt, nullptr);

    const std::string   str = "XXXX";

    for (std::size_t i = 0; i < THREAD_COUNT * 100; ++i)  {
        const double    d = double(i) + 2.5;

        thr_pool.dispatch((! (i % 10)) ? true : false,
                          my_func, i, d, std::cref(str));
    }

    auto    my_func_fut =
        thr_pool.dispatch(false, my_func, 5555, 0.555, std::cref(str));

    std::cout << "my_func Future result: " << my_func_fut.get()
              << std::endl;
    {
        const std::lock_guard<std::mutex>   lock { GLOCK };

        std::cout << "Fourth batch is done ..." << std::endl;

        std::cout << "After Dispatchings; capacity: "
                  << thr_pool.capacity_threads() << " -- "
                  << "available: " << thr_pool.available_threads()
                  << std::endl;
    }

    return;
}

// ----------------------------------------------------------------------------

static void repeating_thread_id()  {

    std::cout << "Running repeating_thread_id() ..." << std::endl;

    constexpr std::size_t   n = 50;
    ThreadPool              thr_pool { THREAD_COUNT };

    for (std::size_t i = 0; i < n; ++i)
        thr_pool.dispatch(
            false,
            [](std::size_t i) -> void {
                const std::lock_guard<std::mutex>   lock { GLOCK };

                std::cout << "From Lambda: " << i
                          << " -- " << std::this_thread::get_id()
                          << std::endl;
            },
            i);
    return;
}

// ----------------------------------------------------------------------------

static void zero_thread_test()  {

    std::cout << "Running zero_thread_test() ..." << std::endl;

    ThreadPool  tp0 (0);
    ThreadPool  tp5 (5);
    ThreadPool  tp10 (10);

    try  {
        tp0.dispatch(false,
                     []() -> void {
                         std::cout << "From Lambda 1" << std::endl;
                     });
        std::cout << "ERROR: We must not print this line 1" << std::endl;
    }
    catch (const std::runtime_error &)  { ; }

    auto    fut = tp0.dispatch(true,
                               []() -> void {
                                   std::cout << "From Lambda 2" << std::endl;
                               });

    fut.get();
    tp0.add_thread(-1);

    tp5.shutdown();
    try  {
        tp5.dispatch(false,
                     []() -> void {
                         std::cout << "From Lambda 3" << std::endl;
                     });
        std::cout << "ERROR: We must not print this line 2" << std::endl;
    }
    catch (const std::runtime_error &)  { ; }

    return;
}

// ----------------------------------------------------------------------------

static void parallel_loop_test()  {

    std::cout << "Running parallel_loop_test() ..." << std::endl;

    constexpr std::size_t       n { 10003 };
    constexpr std::size_t       the_sum { (n * (n + 1)) / 2 };
    std::vector<std::size_t>    vec (n);
    auto                        func =
        [](const auto &begin, const auto &end) -> std::size_t  {
            std::size_t sum { 0 };

            for (auto iter = begin; iter != end; ++iter)
                sum += *iter;
            return (sum);
        };
    ThreadPool                  thr_pool { 5 };

    std::iota(vec.begin(), vec.end(), 1);

    auto    futs = thr_pool.parallel_loop(vec.cbegin(), vec.cend(), func);

    std::size_t result {0};

    for (auto &iter : futs)
        result += iter.get();

    assert(result == the_sum);
}

// ----------------------------------------------------------------------------

int main (int, char *[])  {

    haphazard();
    repeating_thread_id();
    zero_thread_test();
    parallel_loop_test();

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
