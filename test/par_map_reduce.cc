// Hossein Moein
// September 11, 2023
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

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>

using namespace hmthrp;

// -----------------------------------------------------------------------------

static constexpr std::size_t    THREAD_COUNT = 3;

// -----------------------------------------------------------------------------

using WordVector = std::vector<std::string>;

// A 1000 vectors of string vectors
//
std::vector<WordVector> data (1000);

// -----------------------------------------------------------------------------

static void generate_data()  {

    constexpr std::size_t   n_data { 10000 };
    constexpr std::size_t   n_len { 7 };

    // std::string     universe { "QWERTYUIOPASDFGHJKLZXCVBNM" };
    std::string     universe { "QWERTYU" };
    std::mt19937    eng { n_data }; // Always generate the same randoms

    for (auto &iter : data)  {
        iter.reserve(n_data);
        for (std::size_t i = 0; i < n_data; ++i)  {
            std::shuffle(universe.begin(), universe.end(), eng);
            iter.push_back(universe.substr(0, n_len));
        }
    }
}

// -----------------------------------------------------------------------------

using WordCountMap = std::unordered_map<std::string, std::size_t>;

struct  MapFunc  {

    void operator() (const WordVector &words)  {

        std::for_each (words.begin(), words.end(),
                       [this](const std::string &word) -> void {
                           this->wmap_[word]++;
                       });
    }

    const WordCountMap &get_map() const  { return (wmap_); }

private:

    WordCountMap    wmap_ { };
};

// -----------------------------------------------------------------------------

/*
struct  ReduceFunc  {

    void operator() (const WordCountMap &map1, const WordCountMap &map2)  {

        for (const auto &item : map1)
            wmap_[item.first] += item.second;
        for (const auto &item : map2)
            wmap_[item.first] += item.second;
    }

    const WordCountMap &get_map() const { return (wmap_); }

private:

    WordCountMap    wmap_ { };
};
*/

struct  ReduceFunc  {

    WordCountMap
    operator() (WordCountMap &map1, const WordCountMap &map2)  {

        WordCountMap    wmap = std::move(map1);

        for (const auto &item : map2)
            wmap[item.first] += item.second;

        return (wmap);
    }
};

// -----------------------------------------------------------------------------

static void par_map_reduce()  {

    std::cout << "Running par_map_reduce() ..." << std::endl;

    generate_data();

    std::cout << "Done generating data ..." << std::endl;

    ThreadPool  thr_pool (THREAD_COUNT, true, 10);
    const auto  start = std::chrono::high_resolution_clock::now();

    std::vector<std::future<WordCountMap>>  fut_maps;
    const auto                              data_size = data.size();

    fut_maps.reserve(data_size / THREAD_COUNT);
    for (std::size_t i = 0; i < data_size; i += THREAD_COUNT)  {
        fut_maps.push_back(
            thr_pool.dispatch(false,
                              [data_size](std::size_t i) -> WordCountMap  {
                                  MapFunc  map_func;

                                  for (std::size_t j = 0; j < THREAD_COUNT &&
                                           j + i < data_size; ++j)
                                      map_func(data[j + i]);
                                  return (map_func.get_map());
                              },
                              i));
    }

    /*
    ReduceFunc  reduce_func;

    for (std::size_t i = 0; i < fut_maps.size() - 1; i += 2)
        reduce_func(fut_maps[i].get(), fut_maps [i + 1].get());
    for (const auto &item : reduce_func.get_map())
        std::cout << item.first << ": " << item.second << std::endl;
    */
    std::vector<WordCountMap>   maps;

    maps.reserve(fut_maps.size());
    for (auto &item : fut_maps)
        maps.push_back(item.get());

    auto  final_map = std::reduce(/*std::execution::par,*/
                                  maps.begin(), maps.end(),
                                  WordCountMap { },
                                  ReduceFunc { });

    for (const auto &item : final_map)
        std::cout << item.first << ": " << item.second << std::endl;

    const auto  last = std::chrono::high_resolution_clock::now();

    std::cout << "Calculation Time: "
              << "Overall Time: "
              << std::chrono::duration_cast
                     <std::chrono::duration<double>>(last - start).count()
              << std::endl;
    return;
}

int main(int, char *[]) {

    par_map_reduce();

    return (EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
