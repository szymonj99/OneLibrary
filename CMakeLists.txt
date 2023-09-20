cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

set(LIBRARY_NAME OneLibrary)
set(LIBRARY_VERSION 0.0.4)
set(LIBRARY_DESCRIPTION "OneLibrary - Cross-Platform Input Hooking")
set(LIBRARY_HOMEPAGE "https://github.com/szymonj99/OneLibrary")

# Stop complaining about a defined C compiler in a toolchain, as this is only C++.
set(IGNORE_C_COMPILER ${CMAKE_C_COMPILER})
set(IGNORE_C_FLAGS ${CMAKE_C_FLAGS})

option(ONELIBRARY_BUILD_TESTS "Set to `ON` to build tests" ON)
option(ONELIBRARY_OVERWRITE_BUILD_SHARED_LIB "Set to `ON` to overwrite `BUILD_SHARED_LIBS` and to build a SHARED library. Otherwise, library type is decided by CMake's `BUILD_SHARED_LIBS`" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(${LIBRARY_NAME} LANGUAGES CXX VERSION ${LIBRARY_VERSION} DESCRIPTION ${LIBRARY_DESCRIPTION} HOMEPAGE_URL ${LIBRARY_HOMEPAGE})

# In theory, this isn't necessary as we have CMAKE_DISABLE_IN_SOURCE_BUILDS
# Taken from: https://github.com/CharmedBaryon/CommonLibSSE-NG/blob/main/CMakeLists.txt
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed")
endif()

if(CMAKE_CROSSCOMPILING)
    message(WARNING "Cross-compiling is currently not supported. Be careful!")
endif()

include(GNUInstallDirs)
include(cmake/GetCPM.cmake)

function(set_library_type)
    if(ONELIBRARY_OVERWRITE_BUILD_SHARED_LIB)
        set(ONELIBRARY_LIB_TYPE SHARED)
    else()
        if(BUILD_SHARED_LIBS)
            set(ONELIBRARY_LIB_TYPE SHARED)
        else()
            set(ONELIBRARY_LIB_TYPE STATIC)
        endif()
    endif()
endfunction()
set_library_type()
add_library(${PROJECT_NAME} ${ONELIBRARY_LIB_TYPE})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# TODO: Do we want to glob files, or specify them directly?
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/OneLibrary/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS include/OneLibrary/*.h)
# https://github.com/CharmedBaryon/CommonLibSSE-NG/blob/main/cmake/sourcelist.cmake doesn't remove pre-compiled headers, so let's not remove them as well.
set(PUBLIC_HEADER_FILES ${HEADER_FILES})

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES ${SOURCE_FILES} ${HEADER_FILES}
)

# TODO: Is separating the public and private headers better possible? For example: an end-user probably doesn't need something like InputTable.h
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES} ${HEADER_FILES})

# Taken from: https://github.com/CharmedBaryon/CommonLibSSE-NG/blob/main/CMakeLists.txt
# And: https://tttapa.github.io/Pages/Raspberry-Pi/C++-Development/Cpp-Program.html
target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # TODO: Does this need to be `:include` or will this work?
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_precompile_headers(${PROJECT_NAME} PRIVATE include/OneLibrary/PCH.h)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

include(cmake/Util.cmake)
set_threads(${PROJECT_NAME})
set_ipo(${PROJECT_NAME})
set_install(${PROJECT_NAME})
set_compilers(${PROJECT_NAME})

function(set_project_properties)
    # PUBLIC_HEADER will be installed to the user's include/OneLibrary directory.
    set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER_FILES}")
    # Taken from: https://github.com/ChrisThrasher/argon/blob/84fcce3c2c66ae87ba95e65ea587c985c620ce87/CMakeLists.txt#L23
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
endfunction()
set_project_properties()

function(configure_tests)
    # The project is used as a stand-alone, and not as a dependency for a top-level project.
    # If OneLibrary is built as part of a different project, let's not build OneLibrary tests.
    if(CMAKE_PROJECT_NAME STREQUAL ${PROJECT_NAME})
        message(STATUS "${PROJECT_NAME} is the top-level project.")
        if(ONELIBRARY_BUILD_TESTS)
            target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_TESTS=1)
            message(STATUS "Building tests for ${PROJECT_NAME}")
            # enable_testing() needs to be here.
            # Reasoning: https://stackoverflow.com/questions/64099136/visual-studio-2019-test-explorer-did-not-find-c-google-tests
            enable_testing()
            add_subdirectory(tests)
        endif()
    endif()
endfunction()
configure_tests()

function(unset_vars)
    unset(CMAKE_DISABLE_IN_SOURCE_BUILD)
    unset(CMAKE_DISABLE_SOURCE_CHANGES)
    unset(LIBRARY_NAME)
    unset(LIBRARY_VERSION)
    unset(LIBRARY_DESCRIPTION)
    unset(LIBRARY_HOMEPAGE)
    unset(IGNORE_C_COMPILER)
    unset(IGNORE_C_FLAGS)
    unset(PUBLIC_HEADER_FILES)
    unset(ONELIBRARY_LIB_TYPE)
endfunction()
unset_vars()