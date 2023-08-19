add_executable(thrpool_tester thrpool_tester.cc)
target_link_libraries(thrpool_tester PRIVATE Threads::Threads)
target_compile_options(thrpool_tester
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
add_test(NAME thrpool_tester COMMAND thrpool_tester)

