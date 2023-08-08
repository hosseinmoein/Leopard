// Hossein Moein
// August 21, 2007

#include <ThreadPool/ThreadPool.h>

#include <stdexcept>

#include <stdlib.h>

// ----------------------------------------------------------------------------

namespace hmthrp
{

template <typename T, typename A>
ThreadPool<T, A>::
ThreadPool (size_type ini_thr_num, bool timeout_flag, time_type timeout_time)
    : initial_thr_num_ (ini_thr_num >= 0 ? ini_thr_num : 0),
      timeout_time_ (timeout_time),
      timeout_flag_ (timeout_flag)  {

    for (size_type i = 0; i < initial_thr_num_; ++i)  {
        ThreadType  thr (&ThreadPool::thread_routine_, this);

        thr.detach();
    }
}

// ----------------------------------------------------------------------------

template <typename T, typename A>
ThreadPool<T, A>::~ThreadPool ()  {

    if (! shutdown_flag_.load(std::memory_order_relaxed))
        shutdown ();

    if (capacity_threads_.load(std::memory_order_relaxed) != 0)  {
        std::unique_lock<std::mutex>    guard (state_);

        destructor_cvx_.wait (guard);
    }
}

// ----------------------------------------------------------------------------

// NOTE: This routine is _not_ thread safe.
//
template <typename T, typename A>
inline void ThreadPool<T, A>::
terminate_timed_outs_ () noexcept  {

    const size_type thrs_to_shut =
        available_threads_.load(std::memory_order_relaxed) -
        initial_thr_num_ -
        timeouts_pending_.load(std::memory_order_relaxed);

    for (size_type i = 0; i < thrs_to_shut; ++i)  {
        WorkUnit    wu;

        wu.work_type = _timeout_;
        the_queue_.push (wu);
        timeouts_pending_ += 1;
    }

    return;
}

// ----------------------------------------------------------------------------

template <typename T, typename A>
bool ThreadPool<T, A>::
dispatch (class_type *class_ptr,
          thrpool_routine routine,
          arg_type *arg,
          bool immediately) noexcept  {


    if (shutdown_flag_.load(std::memory_order_relaxed))
        // throw Exception ("ThreadPool::dispatch(): "
        //                  "The thread pool is shutdown.");
        return (false);

    if (timeout_flag_.load(std::memory_order_relaxed))
        terminate_timed_outs_ ();

    if (arg)  {
        const WorkUnit  wu(_client_service_,
                           this,
                           class_ptr,
                           routine,
                           nullptr,
                           arg);

        the_queue_.push (wu);
    }
    else  {
        const WorkUnit  wu(_client_service_np_,
                           this,
                           class_ptr,
                           nullptr,
                           routine,
                           nullptr);

        the_queue_.push (wu);
    }

    if (immediately && available_threads_.load(std::memory_order_relaxed) == 0)
        add_thread (1);

    return (true);
}

// ----------------------------------------------------------------------------

template <typename T, typename A>
bool ThreadPool<T, A>::add_thread (size_type thr_num)  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        throw std::runtime_error ("ThreadPool::add_thread(): "
                                  "The thread pool is shutdown.");

    const guard_type    guard (state_);

    if (thr_num < 0)  {
        const size_type thrs_to_shut = ::labs (thr_num);

        if (thrs_to_shut >=
                capacity_threads_.load(std::memory_order_relaxed))  {
            String1K    err;

            err.printf ("ThreadPool::add_thread(): Cannot subtract "
                        "'%d' threads from the pool with capacity '%d'",
                        thrs_to_shut,
                        capacity_threads_.load(std::memory_order_relaxed));
            throw std::runtime_error (err.c_str ());
        }

        for (size_type i = 0; i < thrs_to_shut; ++i)  {
            WorkUnit    wu;

            wu.work_type = _terminate_;
            the_queue_.push (wu);
        }
    }
    else if (thr_num > 0)  {
        for (size_type i = 0; i < thr_num; ++i)  {
            ThreadType  thr (&ThreadPool::thread_routine_, this);

            thr.detach();
        }
    }

    return (true);
}

// ----------------------------------------------------------------------------

template <typename T, typename A>
bool ThreadPool<T, A>::shutdown () noexcept  {

    const guard_type    guard (state_);

    if (! shutdown_flag_.load(std::memory_order_relaxed))  {
        shutdown_flag_.store(true, std::memory_order_relaxed);

        for (size_type i = 0;
             i < capacity_threads_.load(std::memory_order_relaxed); ++i)  {
            WorkUnit    wu;

            wu.work_type = _terminate_;
            the_queue_.push (wu);
        }
    }

    return (true);
}

// ----------------------------------------------------------------------------

template <typename T, typename A>
bool ThreadPool<T, A>::thread_routine_ () noexcept  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        return (true);
    capacity_threads_ += 1;

    time_type   last_busy_time = timeout_flag_ ? ::time (nullptr) : 0;

    while (true)  {
        available_threads_ += 1;

        const WorkUnit  wu = the_queue_.pop_front ();

        available_threads_ -= 1;

        if (wu.work_type == _timeout_)  {
            timeouts_pending_ -= 1;

            if (::time (nullptr) - last_busy_time >= timeout_time_)
                break;
        }
        else if (wu.work_type == _terminate_)  {
            break;
        }
        else  {  // There is a task to do
            class_type  *cp = wu.class_ptr;

            if (wu.work_type == _client_service_)  {
                thrpool_routine rt = wu.the_routine;

                (cp->*rt) (*(wu.arg_ptr));
                delete wu.arg_ptr;
            }
            else if (wu.work_type == _client_service_np_)  {
                thrpool_routine_np  rt = wu.the_routine_np;

                (cp->*rt) ();
            }

            if (timeout_flag_)
                last_busy_time = ::time (nullptr);
        }
    }

    if (--capacity_threads_ == 0)  {
        const guard_type    guard (state_);

        destructor_cvx_.notify_one ();
	}

    return (true);
}

} // namespace hmthrp

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
