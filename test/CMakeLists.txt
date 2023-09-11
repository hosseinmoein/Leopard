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
