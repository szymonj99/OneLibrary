cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

set(LIBRARY_NAME OneLibrary)
set(LIBRARY_VERSION 0.0.4)
set(LIBRARY_DESCRIPTION "OneLibrary - Cross-Platform Input Hooking")
set(LIBRARY_HOMEPAGE "https://github.com/szymonj99/OneLibrary")

# Stop complaining about a defined C compiler in a toolchain, as this is only C++.
set(IGNORE_C_COMPILER ${CMAKE_C_COMPILER})
set(IGNORE_C_FLAGS ${CMAKE_C_FLAGS})

option(ONELIBRARY_BUILD_TESTS "Build the tests" ON)
option(ONELIBRARY_USE_BACKWARDCPP "Compile OneLibrary including Backward-CPP (only active in `Debug` mode)" ON)
option(ONELIBRARY_LIB_TYPE "Can be set to either STATIC or SHARED. Default is not set, meaning the default will be provided by CMake's BUILD_SHARED_LIBS variable.")

if(ONELIBRARY_CLANG_CL_WORKAROUND)
    # TODO: Do not set this for ClangCL-MSVC (Visual Studio generator). Only set for ClangCL when using MinGW build generator.
    set(CMAKE_CXX_FLAGS "-m64")
    # LLVM-mt needs to be set before a call to `project()`, otherwise the ClangCL compiler messes up and becomes a PITA.
    find_program(LLVM_MT_PROGRAM llvm-mt)
    if(LLVM_MT_PROGRAM)
        set(CMAKE_MT "${LLVM_MT_PROGRAM}")
    else()
        message(FATAL_ERROR "llvm-mt is not found. Can't compile with clang-cl without it.")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(${LIBRARY_NAME} LANGUAGES CXX VERSION ${LIBRARY_VERSION} DESCRIPTION ${LIBRARY_DESCRIPTION} HOMEPAGE_URL ${LIBRARY_HOMEPAGE})

if(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
    set(CMAKE_HOST_LINUX TRUE)
endif()

if(NOT ONELIBRARY_LIB_TYPE)
    if(BUILD_SHARED_LIBS)
        set(ONELIBRARY_LIB_TYPE SHARED)
    else()
        set(ONELIBRARY_LIB_TYPE STATIC)
    endif()
endif()
add_library(${PROJECT_NAME} ${ONELIBRARY_LIB_TYPE})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/OneLibrary/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS include/OneLibrary/*.h)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES} PUBLIC ${HEADER_FILES})

include(GNUInstallDirs)
include(GetCPM)

if(ONELIBRARY_USE_BACKWARDCPP AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    CPMAddPackage("gh:bombela/backward-cpp#65a769ffe77cf9d759d801bc792ac56af8e911a3")
    list(APPEND SOURCE_FILES ${BACKWARD_ENABLE})
    add_backward(${PROJECT_NAME})
endif()

if(ONELIBRARY_CLANG_CL_WORKAROUND)
    target_compile_options(${PROJECT_NAME} PUBLIC "/EHsc")
endif()

if(ONELIBRARY_CLANG_WORKAROUND)
    # Clang only, although using it with GCC doesn't hurt.
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

# I'll do more research as to what this actually does.
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        include)

if(CMAKE_HOST_LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE evdev)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "-d")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES UNICODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_BUILD_PARALLEL_LEVEL 8)
set_target_properties(${PROJECT_NAME} PROPERTIES USE_FOLDERS ON)

function(set_ipo project_name)
    # Check inter-procedural optimisation
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        message(STATUS "Inter-procedural Optimisation Enabled")
        set_target_properties(${project_name} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endfunction()
set_ipo(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE=1 UNICODE=1)
# Add final compiler options
if(CMAKE_HOST_WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_WINDOWS=1 WINVER=0x0A00 _WIN32_WINNT=0x0A00 WIN32_LEAN_AND_MEAN=1 NOMINMAX=1)
elseif(CMAKE_HOST_APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_APPLE=1)
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wfatal-errors")
elseif(CMAKE_HOST_LINUX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_LINUX=1)
    target_compile_options(${PROJECT_NAME} PRIVATE "-pthread" "-Wfatal-errors")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_ERROR=1)
endif()

install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION include)

find_program(CCACHE_PROGRAM ccache)
find_program(SCCACHE_PROGRAM sccache)

if(CCACHE_PROGRAM)
    message(STATUS "Found ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

if(SCCACHE_PROGRAM)
    message(STATUS "Found sccache")
    # TODO: This breaks ClangCL build on Windows. https://github.com/mozilla/sccache/issues/1843
    #set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
endif()

# Taken from: https://github.com/ChrisThrasher/argon/blob/84fcce3c2c66ae87ba95e65ea587c985c620ce87/CMakeLists.txt#L23
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)

# The project is used as a stand-alone, and not as a dependency for a top-level project.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "OneLibrary is the top-level project.")
    if(ONELIBRARY_BUILD_TESTS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ONELIBRARY_TESTS=1)
        message(STATUS "Building tests for OneLibrary")
        # enable_testing() needs to be here.
        # Reasoning: https://stackoverflow.com/questions/64099136/visual-studio-2019-test-explorer-did-not-find-c-google-tests
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
