cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

if(CPM_DOWNLOAD_LOCATION)
    # Taken from: https://github.com/cpm-cmake/CPM.cmake/issues/259
    CPMAddPackage(
            NAME Catch2
            GITHUB_REPOSITORY catchorg/Catch2
            VERSION ${Catch2_FIND_VERSION}
            OPTIONS
            "CATCH_BUILD_TESTING OFF"
            "CATCH_BUILD_EXAMPLES OFF"
            "CATCH_BUILD_EXTRA_TESTS OFF"
            "CATCH_BUILD_FUZZERS OFF")
else()
    # Taken from: https://github.com/ChrisThrasher/argon/blob/84fcce3c2c66ae87ba95e65ea587c985c620ce87/tests/FindCatch2.cmake
    include(FetchContent)

    FetchContent_Declare(Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v${Catch2_FIND_VERSION})
    FetchContent_MakeAvailable(Catch2)
endif()

set_target_properties(Catch2 PROPERTIES COMPILE_OPTIONS "" EXPORT_COMPILE_COMMANDS OFF)
set_target_properties(Catch2WithMain PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
get_target_property(Catch2_INCLUDE_DIRS Catch2 INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(Catch2 SYSTEM INTERFACE ${Catch2_INCLUDE_DIRS})
target_compile_features(Catch2 PUBLIC cxx_std_20)
target_compile_features(Catch2WithMain PUBLIC cxx_std_20)
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
