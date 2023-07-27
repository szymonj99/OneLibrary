cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

find_package(Catch2 3.4.0 REQUIRED)

add_subdirectory(manual)
add_subdirectory(automated)
