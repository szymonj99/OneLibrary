cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

# I don't do this:
# I can call `include(cmake/{FILENAME})` and CLion would let me Ctrl + Click to go to the file.
# That's currently not possible if I add the directory and only call `include({FILENAME})`.
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(LIBRARY_NAME OneLibrary)
set(LIBRARY_VERSION 0.0.4)

option(ONELIBRARY_BUILD_TESTS "Build the tests" TRUE)
option(ONELIBRARY_USE_BACKWARDCPP "Compile OneLibrary including Backward-CPP (only active in `Debug` mode)" TRUE)
#option(ONELIBRARY_STATIC_LIB "Make a static library when TRUE. Otherwise build a shared library." FALSE)
# TODO: BUILD_SHARED_LIBS is the option used by CMake. Look into that in the future.

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

include(GNUInstallDirs)

# Sets interprocedural optimisation for a project.
function(set_ipo project_name)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        message(STATUS "Inter-procedural Optimisation Enabled")
        set_target_properties(${project_name} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/OneLibrary/*.cpp ${PROJECT_SOURCE_DIR}/include/OneLibrary/*.h)

# I use this to make development easier.
# This lets a user create a new file and it will be automatically compiled.
# Get all the files needed for the library
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
# Let's only add 'Backward-CPP' if we are running a debug build
if(ONELIBRARY_USE_BACKWARDCPP AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Adding in Backward-CPP to ${PROJECT_NAME}")
    set(Backward_DIR ${PROJECT_SOURCE_DIR}/deps/backward-cpp)
    find_package(Backward CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Backward::Backward)
else()
    #add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
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

set_ipo(${PROJECT_NAME})


# Add final compiler options
if(CMAKE_HOST_WIN32)
#    if(CL)
#        target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
#    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE OS_WINDOWS=1 WINVER=0x0A00 _WIN32_WINNT=0x0A00 _UNICODE=1 UNICODE=1 WIN32_LEAN_AND_MEAN=1 NOMINMAX=1)
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

message(STATUS "Cross Compiling ${CMAKE_CROSSCOMPILING}")

message(STATUS "Test: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Test2: ${CMAKE_CXX_COMPILER_TARGET}")
message(STATUS "Test3: ${WIN32}")
message(STATUS "Test4: ${CMAKE_HOST_WIN32}")

if(ONELIBRARY_BUILD_TESTS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ONELIBRARY_TESTS=1)
    # enable_testing() needs to be here.
    # Reasoning: https://stackoverflow.com/questions/64099136/visual-studio-2019-test-explorer-did-not-find-c-google-tests
    message(STATUS "Building tests for OneLibrary")
    enable_testing()
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

set(OneLibrary_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")
set(OneLibrary_LIBRARIES "")
set(OneLibrary_DEPENDENCIES "Backward")
configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/OneLibraryConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/OneLibraryConfig.cmake"
        @ONLY
)
install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/OneLibraryConfig.cmake"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/${LIBRARY_NAME}"
)
