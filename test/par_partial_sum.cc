// Hossein Moein
// September 13, 2023
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

struct  PartialCalc  {

    using itr_t = typename std::vector<std::size_t>::iterator;
    using fut_t = std::future<std::size_t>;

    std::size_t
    operator() (const itr_t src_begin, const itr_t src_end,
                itr_t dst_begin,
                fut_t *prev_fut)  {

        std::partial_sum(src_begin, src_end, dst_begin);

        const std::size_t   len = std::distance(src_begin, src_end);
        std::size_t         ret_val { *(dst_begin + (len - 1)) };

        // If we were doing this in  place (src == dst), we only needed to
        // add the prev value to the first element of the new block
        //
        if (prev_fut)  {
            const std::size_t   prev_value { prev_fut->get() };

            for (std::size_t i = 0; i < len; ++i)
                *(dst_begin + i) += prev_value;
            ret_val = *(dst_begin + (len - 1));
        }
        return (ret_val);
    }
};

// ----------------------------------------------------------------------------

static void parallel_partial_sum()  {

    std::cout << "Running parallel_partial_sum() ..." << std::endl;

    constexpr std::size_t       n { 1000003 };
    std::vector<std::size_t>    data (n);

    std::iota(data.begin(), data.end(), 1);

    std::vector<std::size_t>                result (n, 0);
    constexpr std::size_t                   block_size { n / THREAD_COUNT };
    std::vector<std::future<std::size_t>>   futs;
    ThreadPool                              thr_pool { THREAD_COUNT };

    futs.reserve(THREAD_COUNT + 1);
    futs.push_back(thr_pool.dispatch(false,
                                     PartialCalc{ },
                                     data.begin(),
                                     data.begin() + block_size,
                                     result.begin(),
                                     nullptr));
    for (std::size_t i = block_size; i < n; i += block_size)  {
        const std::size_t  block_end {
            ((i + block_size) > n) ? n : i + block_size
        };
        // This depends on reserving the right size
        //
        PartialCalc::fut_t *prev_fut = &(futs.back());

        futs.push_back(thr_pool.dispatch(false,
                                         PartialCalc{ },
                                         data.begin() + i,
                                         data.begin() + block_end,
                                         result.begin() + i,
                                         prev_fut));
    }
    futs.back().get();  // wait for the last thread

    constexpr std::size_t   last_sum { (n * (n + 1)) / 2 };

    assert(result.back() == last_sum);
    return;
}

// ----------------------------------------------------------------------------

int main (int, char *[])  {

    parallel_partial_sum();

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
