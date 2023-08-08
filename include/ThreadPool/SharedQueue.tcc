// Hossein Moein
// Copyright (C) 2019-2020 Hossein Moein
// Distributed under the BSD Software License (see file License)
// October 22, 2009

#include <ThreadPool/SharedQueue.h>

// ----------------------------------------------------------------------------

namespace hmthrp
{

template<typename T>
inline void
SharedQueue<T>::push (const value_type &element) noexcept  {

    const AutoLockable  lock (mutex_);
    const bool          was_empty = queue_.empty ();

    queue_.push (element);

    if (was_empty)
        cvx_.notify_all ();
}

// ----------------------------------------------------------------------------

template<typename T>
inline const typename SharedQueue<T>::value_type &
SharedQueue<T>::
front (bool wait_on_front) const  { // throw (SQEmpty)

    std::unique_lock<std::mutex>    ul(mutex_);

    if (queue_.empty ())  {
        if (wait_on_front)
            while (queue_.empty ())
                cvx_.wait (ul);
        else
            throw SQEmpty ();
    }

    return (queue_.front ());
}

// ----------------------------------------------------------------------------

template<typename T>
inline typename SharedQueue<T>::value_type
SharedQueue<T>::
pop_front (bool wait_on_front)  { // throw (SQEmpty)

    std::unique_lock<std::mutex>    ul(mutex_);

    if (queue_.empty ())  {
        if (wait_on_front)
            while (queue_.empty ())
                cvx_.wait (ul);
        else
            throw SQEmpty ();
    }

    const   value_type  value = queue_.front ();

    queue_.pop ();
    return (value);
}

// ----------------------------------------------------------------------------

template<typename T>
inline typename SharedQueue<T>::value_type &
SharedQueue<T>::front (bool wait_on_front)  { // throw (SQEmpty)

    std::unique_lock<std::mutex>    ul(mutex_);

    if (queue_.empty ())  {
        if (wait_on_front)
            while (queue_.empty ())
                cvx_.wait (ul);
        else
            throw SQEmpty ();
    }

    return (queue_.front ());
}

} // namespace hmthrp

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
