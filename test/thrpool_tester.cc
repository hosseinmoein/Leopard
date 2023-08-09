// Hossein Moein
// August 9, 2023

#include <ThreadPool/ThreadPool.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace hmthrp;

//-----------------------------------------------------------------------------

static const size_t THREAD_COUNT = 5;

//-----------------------------------------------------------------------------

class   MyClass  {

    public:

        MyClass ()  { std::cout << "MyClass is Constructed" << std::endl; }
        ~MyClass ()  { std::cout << "MyClass is Destructed" << std::endl; }

    public:

        bool routine ()  {

            struct timespec rqt;

            rqt.tv_sec = 1;
            rqt.tv_nsec = 0;
            ::nanosleep (&rqt, nullptr);

            std::cout << "From routine()\n";

            return (true);
        }
};

//-----------------------------------------------------------------------------

using ThreadPoolType = ThreadPool<MyClass, int>;

//-----------------------------------------------------------------------------

MyClass my_obj;

int main (int, char *[])  {

    ThreadPoolType      thr_pool (THREAD_COUNT, true, 12);

    thr_pool.add_thread (30);

    struct timespec rqt;

    rqt.tv_sec = 11;
    rqt.tv_nsec = 0;
    ::nanosleep (&rqt, nullptr);

    std::cout << "Initial capacity: " << thr_pool.capacity_threads ()
              << " -- available: " << thr_pool.available_threads ()
              << std::endl;

    thr_pool.add_thread (2);

    rqt.tv_sec = 2;
    rqt.tv_nsec = 0;
    ::nanosleep (&rqt, nullptr);  // Give it time to take effect

    std::cout << "After adding 2 threads capacity: "
              << thr_pool.capacity_threads ()
              << " -- available: " << thr_pool.available_threads ()
              << std::endl;

    thr_pool.add_thread (-2);
    ::nanosleep (&rqt, nullptr);  // Give it time to take effect

    std::cout << "After adding -2 threads capacity: "
              << thr_pool.capacity_threads ()
              << " -- available: " << thr_pool.available_threads ()
              << std::endl;

    for (size_t i = 0; i < THREAD_COUNT * 100; ++i)  {
        thr_pool.dispatch (&my_obj, &MyClass::routine,
                           (! (i % 10)) ? true : false);
    }
    ::nanosleep (&rqt, nullptr);

    char    str[1024];

    ::snprintf (str, 1023,
                "After Dispatching threads capacity: %d -- available: %d\n",
                thr_pool.capacity_threads (), thr_pool.available_threads ());
    std::cout << str;

    return (EXIT_SUCCESS);
}

//-----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
