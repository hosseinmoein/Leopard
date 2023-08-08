// Hossein Moein
// October 22, 2009
// Copyright (C) 2019-2020 Hossein Moein
// Distributed under the BSD Software License (see file License)

#pragma once

#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <queue>

// ----------------------------------------------------------------------------

namespace hmthrp
{

class   SQEmpty  { public: inline SQEmpty () throw ()  {   } };

// ----------------------------------------------------------------------------

template<typename T>
class   SharedQueue  {

public:

    using value_type = T;
    using size_type = unsigned int;

    SharedQueue() = default;
    SharedQueue (SharedQueue &&) = default;
    SharedQueue &operator = (SharedQueue &&) = default;
    SharedQueue (const SharedQueue &) = delete;
    SharedQueue &operator = (const SharedQueue &) = delete;

    inline void push (const value_type &element) noexcept;

    inline const value_type &
    front (bool wait_on_front = true) const; // throw (SQEmpty);
    inline value_type &
    front (bool wait_on_front = true); // throw (SQEmpty);

    // NOTE: The following method returns the data by value.
    //       Therefore it is not as efficient as front().
    //       Use it only if you have to.
    //
    inline value_type
    pop_front (bool wait_on_front = true); // throw (SQEmpty);

    inline void pop () noexcept  {

        const AutoLockable  lock (mutex_);

        queue_.pop ();
    }

    inline bool empty () const noexcept  {

        const AutoLockable  lock (mutex_);

        return (queue_.empty ());
    }

    inline size_type size () const noexcept  {

        const AutoLockable  lock (mutex_);

        return (queue_.size ());
    }

private:

    using QueueType = std::queue<value_type, std::deque<value_type>>;
    using AutoLockable = std::lock_guard<std::mutex>;

    mutable std::mutex              mutex_ { };
    mutable std::condition_variable cvx_ { };
    QueueType                       queue_ { };
};

} // namespace hmthrp

// ----------------------------------------------------------------------------

#ifndef HMTHRP_DO_NOT_INCLUDE_TCC_FILES
#    include <HITS/Utils/SharedQueue.tcc>
#endif // HMTHRP_DO_NOT_INCLUDE_TCC_FILES

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
