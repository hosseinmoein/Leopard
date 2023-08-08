// Hossein Moein
// August 21, 2007

#include <ThreadPool/SharedQueue.h>

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

        bool routine_np ()  {

            struct timespec rqt;

            rqt.tv_sec = 1;
            rqt.tv_nsec = 0;
            ::nanosleep (&rqt, nullptr);

            char    str[1024];

            ::printf (str, "From routine_np()\n");
            std::cout << str ();

            return (true);
        }

        bool routine (int &the_i)  {

            struct timespec rqt;

            rqt.tv_sec = 1;
            rqt.tv_nsec = 0;
            ::nanosleep (&rqt, nullptr);

            char    str[1024];

            ::printf (str, "From routine(): The integer is: %d\n", the_i);
            std::cout << str;

            return (true);
        }
};

//-----------------------------------------------------------------------------

using ThreadPoolType = ThreadPool<MyClass, int>;

//-----------------------------------------------------------------------------

int main (int argCnt, char *argVctr [])  {

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

    MyClass my_obj;

    for (size_t i = 0; i < THREAD_COUNT * 100; ++i)  {
        if (i > 250)  {
            thr_pool.dispatch (&my_obj, &MyClass::routine_np,
                               i > THREAD_COUNT ? true : false);
        }
        else  {
            int *the_int = new int (i);

            thr_pool.dispatch (&my_obj, &MyClass::routine, the_int,
                               i > THREAD_COUNT ? true : false);
        }
    }
//    ::nanosleep (&rqt, nullptr);

    char    str[1024];

    ::printf (str, "After Dispatching threads capacity: %d -- available: %d\n",
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
