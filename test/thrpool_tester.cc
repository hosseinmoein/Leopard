// Hossein Moein
// August 9, 2023

#include <ThreadPool/ThreadPool.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

using namespace hmthrp;

// ----------------------------------------------------------------------------

static const std::size_t    THREAD_COUNT = 5;
static std::mutex           GLOCK { };

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

int main (int, char *[])  {

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
        thr_pool.dispatch(false,
                          &MyClass::routine, &my_obj, i);

    auto    my_class_fut =
        thr_pool.dispatch(false, &MyClass::routine, &my_obj, 5555);

    {
        // const std::lock_guard<std::mutex>   lock { GLOCK };

        std::cout << "MyClass Future result: " << my_class_fut.get()
                  << std::endl;
        std::cout << "Second batch is done ..." << std::endl;
    }

    ::nanosleep (&rqt, nullptr);
    for (std::size_t i = 0; i < THREAD_COUNT * 100; ++i)
        thr_pool.dispatch(
            (! (i % 10)) ? true : false,
            [](std::size_t i) -> std::size_t {
                const std::lock_guard<std::mutex>   lock { GLOCK };

                std::cout << "From Lambda: of " << i * i << std::endl;
                return (i * i);
            },
            i);

    {
        const std::lock_guard<std::mutex>   lock { GLOCK };

        std::cout << "Third batch is done ..." << std::endl;
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

    {
        // const std::lock_guard<std::mutex>   lock { GLOCK };

        std::cout << "my_func Future result: " << my_func_fut.get()
                  << std::endl;
        std::cout << "Fourth batch is done ..." << std::endl;

        std::cout << "After Dispatchings; capacity: "
                  << thr_pool.capacity_threads() << " -- "
                  << "available: " << thr_pool.available_threads()
                  << std::endl;
    }

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
