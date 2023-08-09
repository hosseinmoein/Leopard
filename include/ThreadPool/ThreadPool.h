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

#pragma once

#include <ThreadPool/SharedQueue.h>

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <time.h>
#include <thread>

// ----------------------------------------------------------------------------

namespace hmthrp
{

template <typename T, typename A>  // Class Type, Argument
class   ThreadPool  {

public:

    using class_type = T;
    using arg_type = A;
    using size_type = int;
    using time_type = time_t;

   // NOTE: This class takes ownership of the arg pointer and
   //       deletes it when it is doen.
   //
    typedef bool (class_type::*thrpool_routine) ();

    ThreadPool () = delete;
    ThreadPool (const ThreadPool &) = delete;
    ThreadPool &operator = (const ThreadPool &) = delete;

    explicit ThreadPool (size_type ini_thr_num,
                         bool timeout_flag = true,
                         time_type timeout_time = 30 * 60);
    ~ThreadPool ();

public:

    bool dispatch (class_type *class_ptr,
                   thrpool_routine routine,
                   bool immediately = false) noexcept;

    bool add_thread (size_type thr_num);  // Could be positive or negative

    size_type available_threads () const noexcept  {

        return (available_threads_.load(std::memory_order_relaxed));
    }
    size_type capacity_threads () const noexcept  {

        return (capacity_threads_.load(std::memory_order_relaxed));
    }

    bool shutdown () noexcept;

private:

   // This is the routine that is dispatched for each thread
   //
    bool thread_routine_ () noexcept;

private:

    using guard_type = std::lock_guard<std::mutex>;

    enum WORK_TYPE { _undefined_ = 0, _client_service_, _terminate_, _timeout_ };

    struct  WorkUnit  {

        inline WorkUnit () noexcept  {   }
        inline WorkUnit (WORK_TYPE wt,
                         ThreadPool *tpp,
                         class_type *cp,
                         thrpool_routine tr) noexcept
            : work_type (wt),
              thrpool_ptr (tpp),
              class_ptr (cp),
              the_routine (tr)  {   }

        WORK_TYPE       work_type { _undefined_ };
        ThreadPool      *thrpool_ptr { nullptr };
        class_type      *class_ptr { nullptr };
        thrpool_routine the_routine { nullptr };
    };

private:

    using QueueType = SharedQueue<WorkUnit>;
    using ThreadType = std::thread;

    QueueType               the_queue_ { };
    std::atomic<size_type>  available_threads_ { 0 };
    std::atomic<size_type>  capacity_threads_ { 0 };
	std::atomic<size_type>  timeouts_pending_ { 0 };
    std::atomic_bool        shutdown_flag_ { false };
    const size_type         initial_thr_num_;
    const time_type         timeout_time_;
    std::condition_variable destructor_cvx_ { };
    mutable std::mutex      state_ { };
    const bool              timeout_flag_;

   // NOTE: This routine is _not_ thread safe.
   //
    inline void terminate_timed_outs_ () noexcept;
};

} // namespace hmthrp

// ----------------------------------------------------------------------------

#ifndef HMTHRP_DO_NOT_INCLUDE_TCC_FILES
#  include <ThreadPool/ThreadPool.tcc>
#endif // HMTHRP_DO_NOT_INCLUDE_TCC_FILES

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
