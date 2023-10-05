// Hossein Moein
// September 16, 2023
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
#include <iostream>
#include <numeric>
#include <vector>

using namespace hmthrp;

// ----------------------------------------------------------------------------

static constexpr std::size_t    THREAD_COUNT = 5;

// ----------------------------------------------------------------------------

struct  DotProductCalc  {

    using itr_t = typename std::vector<std::size_t>::iterator;

    std::size_t
    operator() (const itr_t src1_begin, const itr_t src1_end,
                itr_t src2_begin)  {

        return (std::inner_product(src1_begin, src1_end, src2_begin,
                                   std::size_t(0)));
    }
};

// ----------------------------------------------------------------------------

static void parallel_dot_product()  {

    std::cout << "Running parallel_dot_product() ..." << std::endl;

    constexpr std::size_t       n { 1000003 };
    std::vector<std::size_t>    data1 (n);
    std::vector<std::size_t>    data2 (n);

    std::iota(data1.begin(), data1.end(), 1);
    std::iota(data2.begin(), data2.end(), data1.back() + 1);

    constexpr std::size_t                   block_size { n / THREAD_COUNT };
    std::vector<std::future<std::size_t>>   futs;
    ThreadPool                              thr_pool { THREAD_COUNT };

    futs.reserve(THREAD_COUNT + 1);
    for (std::size_t i = 0; i < n; i += block_size)  {
        const std::size_t  block_end {
            ((i + block_size) > n) ? n : i + block_size
        };

        futs.push_back(thr_pool.dispatch(false,
                                         DotProductCalc{ },
                                         data1.begin() + i,
                                         data1.begin() + block_end,
                                         data2.begin() + i));
    }

    std::size_t result = 0;

    for (auto &fut : futs)
        result += fut.get();

    assert((result ==
            std::inner_product(data1.begin(), data1.end(), data2.begin(),
                               std::size_t(0))));
    return;
}

// ----------------------------------------------------------------------------

int main (int, char *[])  {

    parallel_dot_product();

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
