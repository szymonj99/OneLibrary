set(TESTS_MANUAL_NAME OneLibraryTestsManual)

add_executable(${TESTS_MANUAL_NAME} main.cpp KeyboardTests.cpp MouseTests.cpp)

target_link_libraries(${TESTS_MANUAL_NAME} PUBLIC OneLibrary Catch2::Catch2WithMain Threads::Threads)

target_compile_features(${TESTS_MANUAL_NAME} PUBLIC cxx_std_20)

catch_discover_tests(${TESTS_MANUAL_NAME})
