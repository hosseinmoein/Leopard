// Hossein Moein
// September 11, 2023

#include <ThreadPool/ThreadPool.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <numeric>

using namespace hmthrp;

// -----------------------------------------------------------------------------

static constexpr std::size_t    THREAD_COUNT = 3;

// -----------------------------------------------------------------------------

using WordVector = std::vector<std::string>;

WordVector  vec1 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "silver"
};
WordVector  vec2 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "gold"
};
WordVector  vec3 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "gold"
};
WordVector  vec4 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "silver"
};
WordVector  vec5 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "FooFoo"
};
WordVector  vec6 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "silverado"
};
WordVector  vec7 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "Busch"
};
WordVector  vec8 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "Busch"
};
WordVector  vec9 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "Busch"
};
WordVector  vec10 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "Busch"
};
WordVector  vec11 {
    "string1", "word2", "xyz", "word2", "word1", "wasd", "wasd", "string1"
    "hype", "word2", "word", "hype", "Hossein"
};

std::vector<WordVector> data {
    vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9, vec10, vec11
};

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
