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

#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <numeric>
#include <stack>
#include <string>

using namespace hmthrp;
using namespace std::chrono;

// ----------------------------------------------------------------------------

// static constexpr std::size_t    THREAD_COUNT = 5;

// ----------------------------------------------------------------------------

struct  ParSorter  {

    using DataType = std::size_t;
    using ContainerType = std::list<DataType>;

    ContainerType do_sort(ContainerType &input_data)  {

        if (input_data.size() < 2)  return (input_data);

        ContainerType   result;

        // The pivot point is the first element
        //
        result.splice(result.begin(), input_data, input_data.begin());

        const DataType  partition_val = *(result.begin());
        auto            divide_point =  // list iterator
            std::partition(input_data.begin(),
                           input_data.end(),
                           [partition_val](const DataType &val) -> bool  {
                               return (val < partition_val);
                           });

        ContainerType   lower_chunk;

        // The pivot point is the first element
        //
        lower_chunk.splice(lower_chunk.begin(),
                           input_data,
                           input_data.begin(),
                           divide_point);

        std::future<ContainerType>  lower_fut =
            thr_pool_.dispatch(false,
                               &ParSorter::do_sort,
                               this,
                               std::ref(lower_chunk));
        ContainerType               higher_chunk = do_sort(input_data);

        result.splice(result.end(), higher_chunk);

        // Run tasks to unblock the recursive tasks
        //
        while (lower_fut.wait_for(std::chrono::seconds(0)) ==
                   std::future_status::timeout)
            thr_pool_.run_task();

        result.splice(result.begin(), lower_fut.get());

        return (result);
    }

private:

    ThreadPool  thr_pool_ { };
};

// --------------------------------------

static void parallel_sort1()  {

    std::cout << "Running parallel_sort1() ..." << std::endl;

    constexpr std::size_t   n { 10'000 };
    std::list<std::size_t>  data (n);

    for (auto &iter : data) iter = ::rand();

    const auto              first = high_resolution_clock::now();
    ParSorter               ps;
    std::list<std::size_t>  sorted_data = ps.do_sort(data);
    auto                    data_end = --(sorted_data.cend());
    const auto              second = high_resolution_clock::now();

    std::cout << "Sorting " << n << " items time: "
              << double(duration_cast<microseconds>(second - first).count()) /
                 1000000.0
              << " secs" << std::endl;

    assert(sorted_data.size() == n);
    for (auto citer = sorted_data.begin(); citer != data_end; )
        assert((*citer <= *(++citer)));
    return;
}

// ----------------------------------------------------------------------------

template<typename I, typename P, std::size_t TH = 500'000>
void sort_data2(I begin, I end, P compare, ThreadPool &thr_pool)  {

    using value_type = typename std::iterator_traits<I>::value_type;
    using fut_type = std::future<void>;

    if (begin >= end) return;

    const std::size_t   data_size = std::distance(begin, end);

    if (data_size > 0)  {
        auto                left_iter = begin;
        auto                right_iter = end;
        bool                is_swapped_left = false;
        bool                is_swapped_right = false;
        const value_type    pivot = *begin;
        auto                fwd_iter = begin + 1;

        while (fwd_iter <= right_iter)  {
            if (compare(*fwd_iter, pivot))  {
                is_swapped_left = true;
                std::iter_swap(left_iter, fwd_iter);
                ++left_iter;
                ++fwd_iter;
            }
            else if (compare(pivot, *fwd_iter))  {
                is_swapped_right = true;
                std::iter_swap(right_iter, fwd_iter);
                --right_iter;
            }
            else ++fwd_iter;
        }

        const bool  do_left =
            is_swapped_left && std::distance(begin, left_iter) > 0;
        const bool  do_right =
            is_swapped_right && std::distance(right_iter, end) > 0;

        if (data_size >= TH)  {
            fut_type    left_fut;
            fut_type    right_fut;

            if (do_left)
                left_fut = thr_pool.dispatch(false,
                                             sort_data2<I, P, TH>,
                                             begin,
                                             left_iter - 1,
                                             compare,
                                             std::ref(thr_pool));
            if (do_right)
                right_fut = thr_pool.dispatch(false,
                                              sort_data2<I, P, TH>,
                                              right_iter + 1,
                                              end,
                                              compare,
                                              std::ref(thr_pool));

            if (do_left)
                while (left_fut.wait_for(std::chrono::seconds(0)) ==
                           std::future_status::timeout)
                    thr_pool.run_task();
            if (do_right)
                while (right_fut.wait_for(std::chrono::seconds(0)) ==
                           std::future_status::timeout)
                    thr_pool.run_task();
        }
        else  {
            if (do_left)
                sort_data2<I, P, TH>(begin, left_iter - 1, compare, thr_pool);

            if (do_right)
                sort_data2<I, P, TH>(right_iter + 1, end, compare, thr_pool);
        }
    }
}

// --------------------------------------

static void parallel_sort2()  {

    std::cout << "Running parallel_sort2() ..." << std::endl;

    constexpr std::size_t   n { 60'000'000 };
    std::vector<double>     data (n);
    ThreadPool              thr_pool { };

    for (auto &iter : data) iter = ::rand();

    const auto  first = high_resolution_clock::now();

    sort_data2(data.begin(), data.end(), std::less<double>{ }, thr_pool);

    const auto  second = high_resolution_clock::now();

    std::cout << "Sorting " << n << " items time: "
              << double(duration_cast<microseconds>(second - first).count()) /
                 1000000.0
              << " secs" << std::endl;

    for (auto citer = data.cbegin(); citer < (data.cend() - 1); ++citer)
        assert((*citer <= *(citer + 1)));
    return;
}

// ----------------------------------------------------------------------------

static void standard_sort()  {

    std::cout << "Running standard_sort() ..." << std::endl;

    constexpr std::size_t   n { 60'000'000 };
    std::vector<double>     data (n);

    for (auto &iter : data) iter = ::rand();

    const auto  first = high_resolution_clock::now();

    std::sort(data.begin(), data.end());

    const auto  second = high_resolution_clock::now();

    std::cout << "Sorting " << n << " items time: "
              << double(duration_cast<microseconds>(second - first).count()) /
                 1000000.0
              << " secs" << std::endl;

    for (auto citer = data.cbegin(); citer < (data.cend() - 1); ++citer)
        assert((*citer <= *(citer + 1)));
    return;
}

// ----------------------------------------------------------------------------

struct  IterativeSort  {

    template<typename I, typename P>
    void do_sort(I first, I last, P compare)  {

        std::stack<I>   stack;

        // push initial values of first and last to stack
        //
        stack.push(first);
        stack.push(last);
 
        // Keep popping from stack while is not empty
        //
        while (! stack.empty())  {
            last = stack.top();
            stack.pop();
            first = stack.top();
            stack.pop();
 
            // Set pivot element at its correct position
            // in sorted range
            //
            const auto  pivot = partition(first, last, compare);
 
            // If there are elements on left side of pivot,
            // then push left side to stack
            //
            if (pivot - 1 > first) {
                stack.push(first);
                stack.push(pivot - 1);
            }
 
            // If there are elements on right side of pivot,
            // then push right side to stack
            //
            if (pivot + 1 < last) {
                stack.push(pivot + 1);
                stack.push(last);
            }
        }
    }

private:

    template<typename I, typename P>
    inline static I partition(I &first, I &last, P compare)  {

        using value_type = typename std::iterator_traits<I>::value_type;

        const value_type    &val = *last;
        auto                i = first - 1;

        for (auto j = first; j < last; j++)  {
            if (compare(*j, val))  {
                ++i;
                std::iter_swap(i, j);
            }
        }
        ++i;
        std::iter_swap(i, last);
        return (i);
    }
};

// --------------------------------------

static void interative_sort()  {

    std::cout << "Running interative_sort() ..." << std::endl;

    constexpr std::size_t   n { 60'000'000 };
    std::vector<double>     data (n);

    for (auto &iter : data) iter = ::rand();

    const auto      first = high_resolution_clock::now();
    IterativeSort   is;

    is.do_sort(data.begin(), data.end(), std::less<double>{ });

    const auto  second = high_resolution_clock::now();

    std::cout << "Sorting " << n << " items time: "
              << double(duration_cast<microseconds>(second - first).count()) /
                 1000000.0
              << " secs" << std::endl;

    for (auto citer = data.cbegin(); citer < (data.cend() - 1); ++citer)
        assert((*citer <= *(++citer)));
    return;
}

// ----------------------------------------------------------------------------

int main (int, char *[])  {

    parallel_sort1();
    parallel_sort2();
    standard_sort();
    interative_sort();

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
