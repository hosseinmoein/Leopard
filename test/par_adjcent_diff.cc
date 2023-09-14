// Hossein Moein
// September 14, 2023
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

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

using namespace hmthrp;

// ----------------------------------------------------------------------------

static constexpr std::size_t    THREAD_COUNT = 5;

// ----------------------------------------------------------------------------

struct  AdjacentDiffCalc  {

    using itr_t = typename std::vector<int>::iterator;

    void
    operator() (itr_t src_begin, itr_t src_end,
                itr_t dst_begin,
                bool first_chunk = false)  {

        if (first_chunk)
            *(dst_begin++) = *(src_begin++);
        for (; src_begin != src_end; ++src_begin)
            *(dst_begin++) = *src_begin - *(src_begin - 1);
    }
};

// ----------------------------------------------------------------------------

static void parallel_adjacent_diff()  {

    std::cout << "Running parallel_adjacent_diff() ..." << std::endl;

    constexpr std::size_t   n { 1000003 };
    std::vector<int>        data (n);

    std::iota(data.begin(), data.end(), 1);
    data[0] = 100;
    data[5] = 50;

    std::vector<int>                result (n, 0);
    constexpr std::size_t           block_size { n / THREAD_COUNT };
    std::vector<std::future<void>>  futs;
    ThreadPool                      thr_pool { THREAD_COUNT };

    futs.reserve(THREAD_COUNT + 1);
    futs.push_back(thr_pool.dispatch(false,
                                     AdjacentDiffCalc{ },
                                     data.begin(),
                                     data.begin() + block_size,
                                     result.begin(),
                                     true));
    for (std::size_t i = block_size; i < n; i += block_size)  {
        const std::size_t  block_end {
            ((i + block_size) > n) ? n : i + block_size
        };

        futs.push_back(thr_pool.dispatch(false,
                                         AdjacentDiffCalc{ },
                                         data.begin() + i,
                                         data.begin() + block_end,
                                         result.begin() + i));
    }

    // Wait for all tasks to finish
    //
    for (auto &fut : futs)
        fut.get();

    assert(result[0] == 100);
    assert(result[1] == -98);
    assert(result[2] == 1);
    assert(result[5] == 45);
    assert(result[6] == -43);
    assert(result[7] == 1);
    assert(result[10] == 1);
    assert(result.back() == 1);
    return;
}

// ----------------------------------------------------------------------------

int main (int, char *[])  {

    parallel_adjacent_diff();

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
