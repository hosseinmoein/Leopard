// Hossein Moein
// August 21, 2007

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
    typedef bool (class_type::*thrpool_routine) (arg_type &);
    typedef bool (class_type::*thrpool_routine_np) ();

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
                   arg_type *arg = nullptr,
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

    enum WORK_TYPE { _undefined_ = 0, _client_service_, 
                     _client_service_np_, _terminate_, _timeout_ };

    struct  WorkUnit  {

        inline WorkUnit () noexcept  {   }
        inline WorkUnit (WORK_TYPE wt,
                         ThreadPool *tpp,
                         class_type *cp,
                         thrpool_routine tr,
                         thrpool_routine_np tr_np,
                         arg_type *ap) noexcept
            : work_type (wt),
              thrpool_ptr (tpp),
              class_ptr (cp),
              the_routine (tr),
              the_routine_np (tr_np),
              arg_ptr (ap)  {   }

        WORK_TYPE           work_type { _undefined_ };
        ThreadPool          *thrpool_ptr { nullptr };
        class_type          *class_ptr { nullptr };
        thrpool_routine     the_routine { nullptr };
        thrpool_routine_np  the_routine_np { nullptr };
        arg_type            *arg_ptr { nullptr };
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
#  include <HITS/Utils/ThreadPool.tcc>
#endif // HMTHRP_DO_NOT_INCLUDE_TCC_FILES

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
