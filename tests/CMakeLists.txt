cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

set(TESTS_NAME OneLibraryTests)

if(ONELIBRARY_USE_VCPKG)
    find_package(Catch2 3 CONFIG REQUIRED)
else()
    # Adding in CPM dependencies
    CPMAddPackage("gh:catchorg/Catch2#0a0ebf50034831718ffc63c6d12abb1fa469d6e9")
endif()

add_executable(OneLibraryTests ThreadsafeQueueTests.cpp)

target_link_libraries(${TESTS_NAME} PRIVATE Catch2::Catch2WithMain)
target_include_directories(${TESTS_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_features(${TESTS_NAME} PRIVATE cxx_std_20)

set_target_properties(${TESTS_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${TESTS_NAME} PROPERTIES CMAKE_CXX_STANDARD 20)
set_target_properties(${TESTS_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED TRUE)
set_target_properties(${TESTS_NAME} PROPERTIES UNICODE TRUE)
set_target_properties(${TESTS_NAME} PROPERTIES CMAKE_BUILD_PARALLEL_LEVEL 8)
set_target_properties(${TESTS_NAME} PROPERTIES USE_FOLDERS TRUE)

#target_compile_definitions(${TESTS_NAME} PRIVATE CATCH_CONFIG_MAIN=1)

include(CTest)
include(Catch)
catch_discover_tests(${TESTS_NAME})
