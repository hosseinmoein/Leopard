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

#include <cstdlib>
#include <ctime>
#include <memory>
#include <stdexcept>

// ----------------------------------------------------------------------------

namespace hmthrp
{

ThreadPool::
ThreadPool(size_type thr_num, bool timeout_flag, time_type timeout_time)
    : timeout_time_ (timeout_time), timeout_flag_ (timeout_flag)  {

    threads_.reserve(thr_num * 2);
    for (size_type i = 0; i < thr_num; ++i)
        threads_.emplace_back(&ThreadPool::thread_routine_, this);
}

// ----------------------------------------------------------------------------

ThreadPool::~ThreadPool()  {

    shutdown();

    const guard_type    guard { state_ };

    for (auto &routine : threads_)
        if (routine.joinable())
            routine.join();
}

// ----------------------------------------------------------------------------

void ThreadPool::terminate_timed_outs_() noexcept  {

    const size_type timeys {
        capacity_threads_.load(std::memory_order_relaxed)
    };

    for (size_type i = 0; i < timeys; ++i)  {
        const WorkUnit  wu { WORK_TYPE::_timeout_ };

        queue_.push(wu);
    }

    return;
}

// ----------------------------------------------------------------------------

template<typename F, typename ... As>
ThreadPool::dispatch_res_t<F, As ...>
ThreadPool::dispatch(bool immediately, F &&routine, As && ... args)  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        throw std::runtime_error("ThreadPool::dispatch(): "
                                 "The thread pool is shutdown.");

    using return_type =
        std::invoke_result_t<std::decay_t<F>, std::decay_t<As> ...>;
    using future_type = ThreadPool::dispatch_res_t<F, As ...>;

    auto            callable  {
        std::make_shared<std::packaged_task<return_type()>>
            (std::bind(std::forward<F>(routine), std::forward<As>(args) ...))
    };
    future_type     return_fut { callable->get_future() };
    const WorkUnit  work_unit {
        WORK_TYPE::_client_service_, [callable]() -> void { (*callable)(); }
    };

    if (immediately && available_threads_.load(std::memory_order_relaxed) == 0)
        add_thread(1);
    queue_.push(work_unit);

    if (timeout_flag_)
        terminate_timed_outs_();
    return (return_fut);
}

// ----------------------------------------------------------------------------

bool ThreadPool::add_thread(size_type thr_num)  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        throw std::runtime_error("ThreadPool::add_thread(): "
                                 "The thread pool is shutdown.");

    if (thr_num < 0)  {
        const size_type shutys { ::abs(thr_num) };

        if (shutys >= capacity_threads_.load(std::memory_order_relaxed))  {
            char    err[1024];

            ::snprintf(err, 1023,
                       "ThreadPool::add_thread(): Cannot subtract "
                       "'%d' threads from the pool with capacity '%d'",
                       shutys,
                       capacity_threads_.load(std::memory_order_relaxed));
            throw std::runtime_error(err);
        }

        for (size_type i = 0; i < shutys; ++i)  {
            const WorkUnit  work_unit { WORK_TYPE::_terminate_ };

            queue_.push(work_unit);
        }
    }
    else if (thr_num > 0)  {
        const guard_type    guard { state_ };

        for (size_type i = 0; i < thr_num; ++i)
            threads_.emplace_back(&ThreadPool::thread_routine_, this);
    }

    return (true);
}

// ----------------------------------------------------------------------------

ThreadPool::size_type
ThreadPool::available_threads() const noexcept  {

    return (available_threads_.load(std::memory_order_relaxed));
}

// ----------------------------------------------------------------------------

ThreadPool::size_type
ThreadPool::capacity_threads() const noexcept  {

    return (capacity_threads_.load(std::memory_order_relaxed));
}

// ----------------------------------------------------------------------------

ThreadPool::size_type
ThreadPool::pending_tasks() const noexcept  {

    return (queue_.size());
}

// ----------------------------------------------------------------------------

bool ThreadPool::shutdown() noexcept  {

    bool    expected { false };

    if (shutdown_flag_.compare_exchange_strong(expected, true,
                                               std::memory_order_relaxed,
                                               std::memory_order_relaxed))  {

        const size_type capacity {
            capacity_threads_.load(std::memory_order_relaxed)
        };

        for (size_type i = 0; i < capacity; ++i)  {
            const WorkUnit  work_unit { WORK_TYPE::_terminate_ };

            queue_.push(work_unit);
        }
    }

    return (true);
}

// ----------------------------------------------------------------------------

bool ThreadPool::run_task() noexcept  {

    try  {
        const WorkUnit  work_unit { queue_.pop_front(false) }; // No wait

       (work_unit.func)();  // Execute the callable
       return (true);
    }
    catch (const SQEmpty &)  { ; }
    return (false);
}

// ----------------------------------------------------------------------------

bool ThreadPool::thread_routine_() noexcept  {

    if (shutdown_flag_.load(std::memory_order_relaxed))
        return (false);

    time_type   last_busy_time { timeout_flag_ ? ::time(nullptr) : 0 };

    ++capacity_threads_;
    while (true)  {
        ++available_threads_;

        const WorkUnit  work_unit { queue_.pop_front() };  // Wait here

        --available_threads_;

        if (work_unit.work_type == WORK_TYPE::_terminate_)  {
            break;
        }
        else if (work_unit.work_type == WORK_TYPE::_timeout_)  {
            if (::time(nullptr) - last_busy_time >= timeout_time_)
                break;
        }
        else if (work_unit.work_type == WORK_TYPE::_client_service_)  {
            (work_unit.func)();  // Execute the callable
            if (timeout_flag_)
                last_busy_time = ::time(nullptr);
        }
    }
    --capacity_threads_;

    return (true);
}

} // namespace hmthrp

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
