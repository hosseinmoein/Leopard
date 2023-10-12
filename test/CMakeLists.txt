add_executable(thrpool_tester thrpool_tester.cc)
target_link_libraries(thrpool_tester PRIVATE Threads::Threads)
target_compile_options(thrpool_tester
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME thrpool_tester COMMAND thrpool_tester)

add_executable(par_sort_tester par_sort_tester.cc)
target_link_libraries(par_sort_tester PRIVATE Threads::Threads)
target_compile_options(par_sort_tester
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME par_sort_tester COMMAND par_sort_tester)

add_executable(par_accumulate_tester par_accumulate_tester.cc)
target_link_libraries(par_accumulate_tester PRIVATE Threads::Threads)
target_compile_options(par_accumulate_tester
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME par_accumulate_tester COMMAND par_accumulate_tester)

add_executable(par_map_reduce par_map_reduce.cc)
target_link_libraries(par_map_reduce PRIVATE Threads::Threads)
target_compile_options(par_map_reduce
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME par_map_reduce COMMAND par_map_reduce)

add_executable(par_partial_sum par_partial_sum.cc)
target_link_libraries(par_partial_sum PRIVATE Threads::Threads)
target_compile_options(par_partial_sum
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME par_partial_sum COMMAND par_partial_sum)

add_executable(par_adjcent_diff par_adjcent_diff.cc)
target_link_libraries(par_adjcent_diff PRIVATE Threads::Threads)
target_compile_options(par_adjcent_diff
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME par_adjcent_diff COMMAND par_adjcent_diff)

add_executable(par_dot_product par_dot_product.cc)
target_link_libraries(par_dot_product PRIVATE Threads::Threads)
target_compile_options(par_dot_product
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME par_dot_product COMMAND par_dot_product)
