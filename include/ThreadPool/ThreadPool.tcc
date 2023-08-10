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
*/

#include <ThreadPool/ThreadPool.h>

#include <stdexcept>
#include <stdlib.h>

// ----------------------------------------------------------------------------

namespace hmthrp
{

template <typename T>
ThreadPool<T>::
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

template <typename T>
ThreadPool<T>::~ThreadPool ()  {

    shutdown ();
    if (capacity_threads_.load(std::memory_order_relaxed) != 0)  {
        std::unique_lock<std::mutex>    guard (state_);

        destructor_cvx_.wait (guard);
    }
}

// ----------------------------------------------------------------------------

template <typename T>
void ThreadPool<T>::terminate_timed_outs_ () noexcept  {

    const size_type thrs_to_shut =
        available_threads_.load(std::memory_order_relaxed) -
        initial_thr_num_ -
        timeouts_pending_.load(std::memory_order_relaxed);

    for (size_type i = 0; i < thrs_to_shut; ++i)  {
        const WorkUnit  wu (WORK_TYPE::_timeout_);

        queue_.push (wu);
        timeouts_pending_ += 1;
    }

    return;
}

// ----------------------------------------------------------------------------

template <typename T>
bool ThreadPool<T>::
dispatch (class_type *class_ptr,
          routine_type routine,
          bool immediately) noexcept  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        // throw Exception ("ThreadPool::dispatch(): "
        //                  "The thread pool is shutdown.");
        return (false);

    const WorkUnit  wu (WORK_TYPE::_client_service_, class_ptr, routine);

    queue_.push (wu);

    if (immediately && available_threads_.load(std::memory_order_relaxed) == 0)
        add_thread (1);

    if (timeout_flag_)
        terminate_timed_outs_();

    return (true);
}

// ----------------------------------------------------------------------------

template <typename T>
bool ThreadPool<T>::add_thread (size_type thr_num)  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        throw std::runtime_error ("ThreadPool::add_thread(): "
                                  "The thread pool is shutdown.");

    const guard_type    guard (state_);

    if (thr_num < 0)  {
        const size_type thrs_to_shut = ::abs (thr_num);

        if (thrs_to_shut >=
                capacity_threads_.load(std::memory_order_relaxed))  {
            char    err[1024];

            ::snprintf(err, 1023,
                       "ThreadPool::add_thread(): Cannot subtract "
                       "'%d' threads from the pool with capacity '%d'",
                       thrs_to_shut,
                       capacity_threads_.load(std::memory_order_relaxed));
            throw std::runtime_error (err);
        }

        for (size_type i = 0; i < thrs_to_shut; ++i)  {
            const WorkUnit  wu (WORK_TYPE::_terminate_);

            queue_.push (wu);
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

template <typename T>
bool ThreadPool<T>::shutdown () noexcept  {

    bool    expected = false;

    if (shutdown_flag_.compare_exchange_strong(expected, true,
                                               std::memory_order_relaxed,
                                               std::memory_order_relaxed))  {
        const guard_type    guard (state_);

        shutdown_flag_.store(true, std::memory_order_relaxed);

        const size_type capacity =
            capacity_threads_.load(std::memory_order_relaxed);

        for (size_type i = 0; i < capacity; ++i)  {
            const WorkUnit  wu (WORK_TYPE::_terminate_);

            queue_.push (wu);
        }
    }

    return (true);
}

// ----------------------------------------------------------------------------

template <typename T>
bool ThreadPool<T>::thread_routine_ () noexcept  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        return (true);
    capacity_threads_ += 1;

    time_type   last_busy_time = timeout_flag_ ? ::time (nullptr) : 0;

    while (true)  {
        available_threads_ += 1;

        const WorkUnit  wu = queue_.pop_front ();  // It can wait here

        available_threads_ -= 1;

        if (wu.work_type == WORK_TYPE::_terminate_)  {
            break;
        }
        else if (wu.work_type == WORK_TYPE::_timeout_)  {
            timeouts_pending_ -= 1;

            if (::time (nullptr) - last_busy_time >= timeout_time_)
                break;
        }
        else if (wu.work_type == WORK_TYPE::_client_service_)  {
            class_type      *cp = wu.class_ptr;
            routine_type    rt = wu.the_routine;

            (cp->*rt) ();
            if (timeout_flag_)
                last_busy_time = ::time (nullptr);
        }
    }

    --capacity_threads_;
    if (capacity_threads_.load(std::memory_order_relaxed) == 0)
        destructor_cvx_.notify_one ();

    return (true);
}

} // namespace hmthrp

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
