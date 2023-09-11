// Hossein Moein
// August 9, 2023

#include <ThreadPool/ThreadPool.h>

#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <numeric>
#include <string>

using namespace hmthrp;

// ----------------------------------------------------------------------------

static constexpr std::size_t    THREAD_COUNT = 5;

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

    ThreadPool  thr_pool_ { THREAD_COUNT };
};

// --------------------------------------

static void parallel_sort()  {

    std::cout << "Running parallel_sort() ..." << std::endl;

    constexpr std::size_t   n { 1003 };
    std::list<std::size_t>  data (n);

    for (auto &iter : data) iter = ::rand();

    ParSorter               ps;
    std::list<std::size_t>  sorted_data = ps.do_sort(data);
    auto                    data_end = --(sorted_data.cend());

    assert(sorted_data.size() == n);
    for (auto citer = sorted_data.begin(); citer != data_end; )
        assert((*citer <= *(++citer)));
    return;
}

// ----------------------------------------------------------------------------

int main (int, char *[])  {

    parallel_sort();

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
