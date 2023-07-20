cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

set(LIBRARY_NAME OneLibrary)
set(LIBRARY_VERSION 0.0.4)

option(ONELIBRARY_BUILD_TESTS "Build the tests" TRUE)
option(ONELIBRARY_USE_BACKWARDCPP "Compile OneLibrary including Backward-CPP (only active in `Debug` mode)" TRUE)
#option(ONELIBRARY_STATIC_LIB "Make a static library when TRUE. Otherwise build a shared library." FALSE)
# TODO: BUILD_SHARED_LIBS is the option used by CMake. Look into that in the future.

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/scripts/buildsystems/vcpkg.cmake")
if(CMAKE_HOST_WIN32)
    message(STATUS "We are here")
    message(STATUS ${CMAKE_CXX_COMPILER_ID})
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Clang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            message(STATUS "Clang-cl")
            # using clang with clang-cl front end
            message(STATUS "Using clang with clang-cl MSVC front-end")

            find_program(LLVM_MT_PROGRAM llvm-mt)
            if(LLVM_MT_PROGRAM)
                message(STATUS "Found llvm-mt")
                set(CMAKE_MT "${LLVM_MT_PROGRAM}")
            else()
                message(WARNING "Found Clang-cl as the compiler front-end, but didn't find llvm-mt which is necessary. Compiling is likely to fail.")
            endif()
        endif()
    endif()
endif()

find_program(LLVM_MT_PROGRAM llvm-mt)
if(LLVM_MT_PROGRAM)
    message(STATUS "Found llvm-mt")
    set(CMAKE_MT "${LLVM_MT_PROGRAM}")
else()
    message(WARNING "Found Clang-cl as the compiler front-end, but didn't find llvm-mt which is necessary. Compiling is likely to fail.")
endif()
#set(CMAKE_MT "C:/Users/szymon/scoop/apps/mingw-winlibs-llvm-ucrt/current/bin/llvm-mt.exe")
#set(CMAKE_MT "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64/mt.exe")

find_program(CCACHE_PROGRAM ccache)
find_program(SCCACHE_PROGRAM sccache)

if(CCACHE_PROGRAM)
    message(STATUS "Found ccache")
    #set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    #set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()

if(SCCACHE_PROGRAM)
    message(STATUS "Found sccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    #set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()

#find_program(CLANG_CL_PROGRAM clang-cl)
#if(CLANG_CL_PROGRAM)
#    message(STATUS "Found clang-cl")
#    set(CMAKE_CXX_COMPILER "${CLANG_CL_PROGRAM}")
#endif()

project(${LIBRARY_NAME} LANGUAGES CXX VERSION ${LIBRARY_VERSION} DESCRIPTION "OneLibrary - Cross-Platform Input Hooking" HOMEPAGE_URL https://github.com/szymonj99/OneLibrary)

include(CMakePackageConfigHelpers)
include(CPack)
include(GenerateExportHeader)

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

# Get CPM
include(GetCPMConfig)
include(GNUInstallDirs)

#message(STATUS ${CMAKE_LINKER})

# Although GLOB'ing files is an anti-pattern (https://cliutils.gitlab.io/modern-cmake/chapters/intro/dodonot.html)
# I use it to not have to manually specify files which can quickly become tedious.
# TODO: Depending on the target platform, we can exclude files from these lists if they are for a different platform, even if all files are guarded by `#ifdef OS_{PLATFORM}`
#file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/OneLibrary/*.cpp)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/OneLibrary/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/OneLibrary/*.h)
file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/OneLibrary/*.h)


# Let's only add 'Backward-CPP' if we are running a debug build
# TODO: Make this better.
if(ONELIBRARY_USE_BACKWARDCPP AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Adding in Backward-CPP to ${PROJECT_NAME}")
    CPMAddPackage("gh:bombela/backward-cpp#90398eef20f4e7e0e939322d7e84f9c52078a325")
    list(APPEND SOURCE_FILES ${BACKWARD_ENABLE})
    #add_library(${PROJECT_NAME} ${ONELIBRARY_LIB_TYPE} ${SOURCE_FILES})
    #add_library(${PROJECT_NAME} STATIC)
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
    add_backward(${PROJECT_NAME})
else()
    #add_library(${PROJECT_NAME} ${ONELIBRARY_LIB_TYPE} ${SOURCE_FILES})
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
    #add_library(${PROJECT_NAME} STATIC)
endif()

#target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
#target_include_directories(${PROJECT_NAME} PUBLIC ${PUBLIC_HEADERS})

#add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
#        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
#        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

# I'll do more research as to what this actually does.
#target_include_directories(${PROJECT_NAME}
#        PUBLIC
#        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
#        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#        ${PROJECT_SOURCE_DIR}/include)

if(CMAKE_HOST_LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE evdev)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "-d")

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# I guess this is public so that projects using it as a dependency can't change the standard.
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Taken from: https://github.com/ChrisThrasher/argon/blob/84fcce3c2c66ae87ba95e65ea587c985c620ce87/CMakeLists.txt#L23
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)

#generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/OneLibrary/Export.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES UNICODE TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_BUILD_PARALLEL_LEVEL 8)
set_target_properties(${PROJECT_NAME} PROPERTIES USE_FOLDERS TRUE)

# Check inter-procedural optimisation
#include(CheckIPOSupported)
#check_ipo_supported(RESULT result)
#if(result)
#    message(STATUS "Inter-procedural Optimisation Enabled")
#    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
#endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE=1 UNICODE=1)

#if(${CMAKE_CROSSCOMPILING})
#    message(WARNING "Cross-compiling is currently _NOT_ supported. You are treading on dangerous grounds, where compiler definitions are 100% likely to be incorrect.")
#endif()

#macro(configure_compiler project_name)
#endmacro()

# Add final compiler options
# TODO: Make this better.
if(CMAKE_HOST_WIN32)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # GCC
        message(STATUS "GNU GCC")
        #target_compile_definitions(${PROJECT_NAME} PRIVATE "-stdlib=libstdc++")
        #target_compile_definitions(${PROJECT_NAME} PRIVATE "-stdlib=libc++")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(STATUS "MSVC")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Clang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            message(STATUS "Clang with Clang-cl front-end")

            # using clang with clang-cl front end
        elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
            message(STATUS "Clang with regular front-end")
            # using clang with regular front end
        endif()
    else()
        message(WARNING "Unknown compiler. Compiling may fail, or have incorrectly set defines/flags. Try using GNU GCC (MinGW), Clang (MinGW) or MSVC (Visual Studio) instead.")
    endif()
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

#install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION ${PROJECT_SOURCE_DIR}/include)

#install(DIRECTORY include/ ${PROJECT_BINARY_DIR}/include/
#        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
#
#install(TARGETS ${PROJECT_NAME} EXPORT OneLibrary-targets
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
#        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
#
#install(EXPORT OneLibrary-targets
#        NAMESPACE OneLibrary::
#        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

#write_basic_package_version_file(OneLibraryConfigVersion.cmake COMPATIBILITY SameMajorVersion)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OneLibraryConfig.cmake
#        ${CMAKE_CURRENT_BINARY_DIR}/OneLibraryConfigVersion.cmake
#        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

# The project is used as a stand-alone, and not as a dependency for a top-level project.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "OneLibrary is the top-level project")
    if(ONELIBRARY_BUILD_TESTS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ONELIBRARY_TESTS=1)
        message(STATUS "Building tests for OneLibrary")
        # enable_testing() needs to be here.
        # Reasoning: https://stackoverflow.com/questions/64099136/visual-studio-2019-test-explorer-did-not-find-c-google-tests
        enable_testing()
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
    endif()
endif()

message(STATUS ${CMAKE_CXX_FLAGS})

#set(CMAKE_CXX_FLAGS "-m64")
message(STATUS ${CMAKE_CXX_FLAGS})
message(STATUS ${CMAKE_CXX_COMPILE_FEATURES})
set(CMAKE_VERBOSE_MAKEFILE TRUE)
string(REPLACE "/wd4668" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")