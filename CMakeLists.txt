cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

# I don't do this:
# I can call `include(cmake/{FILENAME})` and CLion would let me Ctrl + Click to go to the file.
# That's currently not possible if I add the directory and only call `include({FILENAME})`.
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/scripts/buildsystems/vcpkg.cmake")

option(ONELIBRARY_BUILD_TESTS "Build the tests" TRUE)
option(ONELIBRARY_USE_BACKWARDCPP "Compile OneLibrary including Backward-CPP (only active in `Debug` mode)" TRUE)

project(OneLibrary LANGUAGES CXX VERSION 0.0.4 DESCRIPTION "Cross-Platform Input Hooking Library")

# TODO: Allow a developer to override this option.
if(BUILD_SHARED_LIBS)
    message(STATUS "OneLibrary will be built as a shared library")
    set(ONELIBRARY_LIB_TYPE SHARED)
else()
    message(STATUS "OneLibrary will be built as a static library")
    set(ONELIBRARY_LIB_TYPE STATIC)
endif()

if(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
    set(CMAKE_HOST_LINUX TRUE)
endif()

include(GNUInstallDirs)

# Sets interprocedural optimisation for a project.
macro(set_ipo project_name)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        message(STATUS "Inter-procedural Optimisation Enabled")
        set_target_properties(${project_name} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endmacro()

# I use this to make development easier.
# This lets a user create a new file and it will be automatically compiled.
# Get all the files needed for the library
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/OneLibrary/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/OneLibrary/*.h)

add_library(${PROJECT_NAME} ${ONELIBRARY_LIB_TYPE})
target_sources(${PROJECT_NAME}
        PUBLIC ${HEADER_FILES}
        PRIVATE ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${HEADER_FILES})
macro(use_backward project_name)
    # Let's only add 'Backward-CPP' if we are running a debug build
    if(ONELIBRARY_USE_BACKWARDCPP AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Adding in Backward-CPP to ${project_name}")
        set(Backward_DIR ${PROJECT_SOURCE_DIR}/extern/backward-cpp)
        find_package(Backward CONFIG REQUIRED)
        target_link_libraries(${project_name} PRIVATE Backward::Backward)
    endif()
endmacro()

use_backward(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>)

if(CMAKE_HOST_LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE evdev)
endif()

file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/*.h)

macro(configure_properties project_name)
    set_target_properties(${project_name} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
    set_target_properties(${project_name} PROPERTIES DEBUG_POSTFIX "-d")
    set_target_properties(${project_name} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${project_name} PROPERTIES CMAKE_CXX_STANDARD 20)
    set_target_properties(${project_name} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED TRUE)
    set_target_properties(${project_name} PROPERTIES UNICODE TRUE)
    set_target_properties(${project_name} PROPERTIES CMAKE_BUILD_PARALLEL_LEVEL 8)
    set_target_properties(${project_name} PROPERTIES USE_FOLDERS TRUE)
endmacro()

configure_properties(${PROJECT_NAME})
set_ipo(${PROJECT_NAME})

# Add final compiler options
macro(setup_compiler_options project_name)
    target_compile_features(${project_name} PRIVATE cxx_std_20)
    # We should:
    # - Check if cross-compiling (do not only check the host platform)
    # - Try using cross-platform CMake options rather than hard-coding options
    if(CMAKE_HOST_WIN32)
        target_compile_definitions(${project_name} PRIVATE OS_WINDOWS=1 WINVER=0x0A00 _WIN32_WINNT=0x0A00 _UNICODE=1 UNICODE=1 WIN32_LEAN_AND_MEAN=1 NOMINMAX=1)
    elseif(CMAKE_HOST_APPLE)
        target_compile_definitions(${project_name} PRIVATE OS_APPLE=1 _UNICODE=1 UNICODE=1)
        target_compile_options(${project_name} PRIVATE "-Wfatal-errors")
    elseif(CMAKE_HOST_LINUX)
        target_compile_definitions(${project_name} PRIVATE OS_LINUX=1 _UNICODE=1 UNICODE=1)
        target_compile_options(${project_name} PRIVATE "-pthread" "-Wfatal-errors")
    else()
        target_compile_definitions(${project_name} PRIVATE OS_ERROR=1)
    endif()
endmacro()

setup_compiler_options(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        )
install(TARGETS ${PROJECT_NAME}
        EXPORT OneLibraryTargets
        DESTINATION lib)
install(FILES ${PUBLIC_HEADERS} DESTINATION include)

macro(setup_tests project_name)
    if(ONELIBRARY_BUILD_TESTS)
        target_compile_definitions(${project_name} PRIVATE ONELIBRARY_TESTS=1)
        # enable_testing() needs to be here.
        # Reasoning: https://stackoverflow.com/questions/64099136/visual-studio-2019-test-explorer-did-not-find-c-google-tests
        message(STATUS "Building tests for OneLibrary")
        enable_testing()
        add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
    endif()
endmacro()

setup_tests(${PROJECT_NAME})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/OneLibrary/Version.h)

set(ONELIBRARY_CMAKE_CONFIG_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Catch2")

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_LIST_DIR}/cmake/OneLibraryConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/OneLibraryConfig.cmake"
        INSTALL_DESTINATION lib/OneLibrary
)

include(CPack)
