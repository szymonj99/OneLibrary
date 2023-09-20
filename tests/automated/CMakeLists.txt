set(TESTS_AUTOMATED_NAME OneLibraryTestsAutomated)

add_executable(${TESTS_AUTOMATED_NAME} ThreadsafeQueueTests.cpp InputTableTests.cpp)

target_link_libraries(${TESTS_AUTOMATED_NAME} PUBLIC OneLibrary Catch2::Catch2 Threads::Threads)

target_compile_definitions(OneLibrary PUBLIC UNICODE _UNICODE)

target_compile_features(${TESTS_AUTOMATED_NAME} PUBLIC cxx_std_20)

catch_discover_tests(${TESTS_AUTOMATED_NAME})
