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

    // Make sure at least one thread is running before we exit the constructor
    //
    if (thr_num > 0)
        while (capacity_threads() == 0)
            std::this_thread::yield();
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

    const size_type timeys { capacity_threads() };

    for (size_type i = 0; i < timeys; ++i)  {
        const WorkUnit  work_unit { WORK_TYPE::_timeout_ };

        global_queue_.push(work_unit);
    }

    return;
}

// ----------------------------------------------------------------------------

template<typename F, typename ... As>
ThreadPool::dispatch_res_t<F, As ...>
ThreadPool::dispatch(bool immediately, F &&routine, As && ... args)  {

    if (is_shutdown() || (capacity_threads() == 0 && ! immediately))
        throw std::runtime_error("ThreadPool::dispatch(): "
                                 "Thread-pool has 0 thread capacity.");

    using task_return_t =
        std::invoke_result_t<std::decay_t<F>, std::decay_t<As> ...>;
    using future_t = dispatch_res_t<F, As ...>;

    auto            callable  {
        std::make_shared<std::packaged_task<task_return_t()>>
            (std::bind(std::forward<F>(routine), std::forward<As>(args) ...))
    };
    future_t        return_fut { callable->get_future() };
    const WorkUnit  work_unit {
        WORK_TYPE::_client_service_, [callable]() -> void { (*callable)(); }
    };

    if (immediately && available_threads() == 0)
        add_thread(1);

    if (local_queue_)  // Is this one of the pool threads
        local_queue_->push(work_unit);
    else
        global_queue_.push(work_unit);

    if (timeout_flag_)
        terminate_timed_outs_();
    return (return_fut);
}

// ----------------------------------------------------------------------------

template<typename F, typename I, typename ... As>
ThreadPool::loop_res_t<F, I>
ThreadPool::parallel_loop(I begin, I end, F &&routine, As && ... args)  {

    using task_return_t =
        std::invoke_result_t<std::decay_t<F>,
                             std::decay_t<I>,
                             std::decay_t<I>,
                             std::decay_t<As> ...>;
    using future_t = std::future<task_return_t>;

    const size_type         n { std::distance(begin, end) };
    const size_type         block_size { n / capacity_threads() };
    std::vector<future_t>   ret;

    ret.reserve(capacity_threads() + 1);
    for (size_type i = 0; i < n; i += block_size)  {
        const size_type block_end {
            ((i + block_size) > n) ? n : i + block_size
        };

        ret.push_back(dispatch(false,
                               routine,
                               begin + i,
                               begin + block_end,
                               std::forward<As>(args) ...));
    }

    return (ret);
}

// ----------------------------------------------------------------------------

bool ThreadPool::add_thread(size_type thr_num)  {

    if (is_shutdown())
        throw std::runtime_error("ThreadPool::add_thread(): "
                                 "The thread pool is shutdown.");

    if (thr_num < 0)  {
        const size_type shutys { ::abs(thr_num) };

        if (shutys > capacity_threads())  {
            char    err[1024];

            ::snprintf(err, 1023,
                       "ThreadPool::add_thread(): Cannot subtract "
                       "'%ld' threads from the pool with capacity '%ld'",
                       shutys, capacity_threads());
            throw std::runtime_error(err);
        }

        for (size_type i = 0; i < shutys; ++i)  {
            const WorkUnit  work_unit { WORK_TYPE::_terminate_ };

            global_queue_.push(work_unit);
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

bool
ThreadPool::is_shutdown() const noexcept  {

    return (shutdown_flag_.load(std::memory_order_relaxed));
}

// ----------------------------------------------------------------------------

ThreadPool::size_type
ThreadPool::pending_tasks() const noexcept  {

    return (global_queue_.size());
}

// ----------------------------------------------------------------------------

bool ThreadPool::shutdown() noexcept  {

    bool    expected { false };

    if (shutdown_flag_.compare_exchange_strong(expected, true,
                                               std::memory_order_relaxed,
                                               std::memory_order_relaxed))  {
        const size_type capacity { capacity_threads() };

        for (size_type i = 0; i < capacity; ++i)  {
            const WorkUnit  work_unit { WORK_TYPE::_terminate_ };

            global_queue_.push(work_unit);
        }
    }

    return (true);
}

// ----------------------------------------------------------------------------

bool ThreadPool::run_task() noexcept  {

    try  {
        WorkUnit    work_unit;

        if (local_queue_ && ! local_queue_->empty())  {  // Local pool thread
            work_unit = local_queue_->front();
            local_queue_->pop();
        }
        else
            work_unit = global_queue_.pop_front(false); // No wait

        if (work_unit.work_type == WORK_TYPE::_client_service_)  {
            (work_unit.func)();  // Execute the callable
            return (true);
        }
        else if (work_unit.work_type != WORK_TYPE::_undefined_)
            global_queue_.push(work_unit);  // Put it back
    }
    catch (const SQEmpty &)  { ; }
    return (false);
}

// ----------------------------------------------------------------------------

bool ThreadPool::thread_routine_() noexcept  {

    if (is_shutdown())
        return (false);

    time_type   last_busy_time { timeout_flag_ ? ::time(nullptr) : 0 };

    ++capacity_threads_;
    local_queue_.reset(new LocalQueueType);
    while (true)  {
        ++available_threads_;

        WorkUnit    work_unit;

        // First empty the locla queue
        //
        if (! local_queue_->empty())  {  // No sync needed since it is local
            work_unit = local_queue_->front();
            local_queue_->pop();
        }
        else
            work_unit = global_queue_.pop_front();  // Wait here

        --available_threads_;

        if (work_unit.work_type == WORK_TYPE::_terminate_)  {
            break;
        }
        else if (work_unit.work_type == WORK_TYPE::_timeout_)  {
            if (::time(nullptr) - last_busy_time >= timeout_time_)
                break;
        }
        else if (work_unit.work_type == WORK_TYPE::_client_service_)  {
            if (timeout_flag_)
                last_busy_time = ::time(nullptr);
            (work_unit.func)();  // Execute the callable
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