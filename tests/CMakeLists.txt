cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

if(ONELIBRARY_USE_VCPKG)
    find_package(Catch2 3 CONFIG REQUIRED)
else()
    # Adding in CPM dependencies
    CPMAddPackage("gh:catchorg/Catch2#0a0ebf50034831718ffc63c6d12abb1fa469d6e9")
endif()

add_subdirectory(manual)
add_subdirectory(automated)
