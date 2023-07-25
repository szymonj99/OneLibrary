# TODO: Figure out why using CPM results in include(Catch) not working.
#if(CPM_DOWNLOAD_LOCATION)
#    CPMAddPackage(
#            NAME Catch2
#            GITHUB_REPOSITORY catchorg/Catch2
#            VERSION ${Catch2_FIND_VERSION}
#            OPTIONS
#            "CATCH_BUILD_TESTING OFF"
#            "CATCH_BUILD_EXAMPLES OFF"
#            "CATCH_BUILD_EXTRA_TESTS OFF"
#            "CATCH_BUILD_FUZZERS OFF")
#else()
#    # Taken from: https://github.com/ChrisThrasher/argon/blob/84fcce3c2c66ae87ba95e65ea587c985c620ce87/tests/FindCatch2.cmake
#    include(FetchContent)
#
#    FetchContent_Declare(Catch2
#            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#            GIT_TAG v${Catch2_FIND_VERSION})
#    FetchContent_MakeAvailable(Catch2)
#endif()

# Taken from: https://github.com/ChrisThrasher/argon/blob/84fcce3c2c66ae87ba95e65ea587c985c620ce87/tests/FindCatch2.cmake
include(FetchContent)

FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v${Catch2_FIND_VERSION})
FetchContent_MakeAvailable(Catch2)

set_target_properties(Catch2 PROPERTIES COMPILE_OPTIONS "" EXPORT_COMPILE_COMMANDS OFF)
set_target_properties(Catch2WithMain PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
get_target_property(CATCH2_INCLUDE_DIRS Catch2 INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(Catch2 SYSTEM INTERFACE ${CATCH2_INCLUDE_DIRS})
target_compile_features(Catch2 PRIVATE cxx_std_20)

# TODO: Figure out if there is a better way of dealing with this.
if(ONELIBRARY_CLANG_CL_WORKAROUND)
    target_compile_options(Catch2 PUBLIC "/EHsc")
endif()
