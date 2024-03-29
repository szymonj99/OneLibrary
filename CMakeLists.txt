cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

set(LIBRARY_NAME OneLibrary)
set(LIBRARY_VERSION 0.0.4)

option(ONELIBRARY_BUILD_TESTS "Build the tests" TRUE)
option(ONELIBRARY_USE_BACKWARDCPP "Compile OneLibrary including Backward-CPP (only active in `Debug` mode)" TRUE)
#option(ONELIBRARY_STATIC_LIB "Make a static library when TRUE. Otherwise build a shared library." FALSE)
# TODO: BUILD_SHARED_LIBS is the option used by CMake. Look into that in the future.
option(ONELIBRARY_USE_VCPKG "Use VCPKG to manage dependencies rather than the default CPM. Note: Some dependencies may still be downloaded with CPM if they are not available through VCPKG" TRUE)

if(ONELIBRARY_USE_VCPKG)
    if(NOT DEFINED ENV{VCPKG_ROOT})
        message(FATAL_ERROR "VCPKG_ROOT is not set!")
    else()
        file(REAL_PATH $ENV{VCPKG_ROOT} VCPKG_ROOT)
        message(STATUS "VCPKG_ROOT is defined as: $ENV{VCPKG_ROOT} (Real Path: ${VCPKG_ROOT})")
    endif()
    # This needs to be done before calling project()
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using VCPKG Toolchain from: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(${LIBRARY_NAME} LANGUAGES CXX VERSION ${LIBRARY_VERSION} DESCRIPTION "OneLibrary - Cross-Platform Input Hooking")

#if(ONELIBRARY_STATIC_LIB)
#    message(STATUS "OneLibrary will be built as a static library")
#    set(ONELIBRARY_LIB_TYPE STATIC)
#else()
#    message(STATUS "OneLibrary will be built as a shared library")
#    set(ONELIBRARY_LIB_TYPE SHARED)
#endif()

if(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
    set(CMAKE_HOST_LINUX TRUE)
endif()

# https://stackoverflow.com/questions/70462926/how-to-check-my-cmake-using-debug-or-release
if (NOT CMAKE_BUILD_TYPE)
    message("Build Type not set, defaulting to Debug...")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Get CPM
include(cmake/GetCPMConfig.cmake)
include(GNUInstallDirs)

# I use this to make development easier.
# This lets a user create a new file and it will be automatically compiled.
# Get all the files needed for the library
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/OneLibrary/*.cpp ${PROJECT_SOURCE_DIR}/include/OneLibrary/*.h)

# Let's only add 'Backward-CPP' if we are running a debug build
if(ONELIBRARY_USE_BACKWARDCPP AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Adding in Backward-CPP to ${PROJECT_NAME}")
    CPMAddPackage("gh:bombela/backward-cpp#90398eef20f4e7e0e939322d7e84f9c52078a325")
    list(APPEND SOURCE_FILES ${BACKWARD_ENABLE})
    #add_library(${PROJECT_NAME} ${ONELIBRARY_LIB_TYPE} ${SOURCE_FILES})
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
    add_backward(${PROJECT_NAME})
else()
    #add_library(${PROJECT_NAME} ${ONELIBRARY_LIB_TYPE} ${SOURCE_FILES})
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
endif()

# I'll do more research as to what this actually does.
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${PROJECT_SOURCE_DIR}/include)

if(CMAKE_HOST_LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE evdev)
endif()

file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/*.h)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "Debug")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES UNICODE TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_BUILD_PARALLEL_LEVEL 8)
set_target_properties(${PROJECT_NAME} PROPERTIES USE_FOLDERS TRUE)

# Check inter-procedural optimisation
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    message(STATUS "Inter-procedural Optimisation Enabled")
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Add final compiler options
if(CMAKE_HOST_WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_WINDOWS=1 WINVER=0x0A00 _WIN32_WINNT=0x0A00 _UNICODE=1 UNICODE=1 WIN32_LEAN_AND_MEAN=1 NOMINMAX=1)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
elseif(CMAKE_HOST_APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_APPLE=1 _UNICODE=1 UNICODE=1)
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wfatal-errors")
elseif(CMAKE_HOST_LINUX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_LINUX=1 _UNICODE=1 UNICODE=1)
    target_compile_options(${PROJECT_NAME} PRIVATE "-pthread" "-Wfatal-errors")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_ERROR=1)
endif()

#install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION ${PROJECT_SOURCE_DIR}/include)

if(ONELIBRARY_BUILD_TESTS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ONELIBRARY_TESTS=1)
    # enable_testing() needs to be here.
    # Reasoning: https://stackoverflow.com/questions/64099136/visual-studio-2019-test-explorer-did-not-find-c-google-tests
    message(STATUS "Building tests for OneLibrary")
    enable_testing()
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()
