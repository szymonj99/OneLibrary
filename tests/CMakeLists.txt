cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

find_package(Catch2 3 CONFIG REQUIRED)

add_subdirectory(manual)
add_subdirectory(automated)
