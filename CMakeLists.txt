cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

set(LIBRARY_NAME OneLibrary)
set(LIBRARY_VERSION 0.0.1)

project(${LIBRARY_NAME} LANGUAGES CXX VERSION ${LIBRARY_VERSION} DESCRIPTION "OneLibrary - Cross-Platform Input Hooking")

option(PACKAGE_TESTS "Build the tests" FALSE)

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

set(LINKER_LANGUAGE CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE FALSE) # I disabled this as I felt it wasn't necessary.
set(BUILD_SHARED_LIBS FALSE) # I want this to be a self-contained application at first, so all the dependencies are included in it.
set(UNICODE TRUE)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

# Adding in CPM dependencies
# TODO: Should this go here or in OneControl?
## Argument parsing
### Which one should I pick?
#CPMAddPackage("gh:Taywee/args#11016849db5fdeceb5bf3550ac7f0ada1ce2419c")
#CPMAddPackage("gh:p-ranav/argparse#af0f7cb25f9144499854e1874410fbc288b36fa4")
#CPMAddPackage("gh:badaix/popl#bda5f43099d67419089a44c9e54474e4998a9a26")
#CPMAddPackage("gh:SFML/sfml#fd1435d1c04750893369067d75e0f667a88e0f6c")
#CPMAddPackage("gh:fmtlib/fmt#7e5a9595648f4643cd81eed83792ca93bf3b75c3")

# I use this to make development easier.
# This lets a user create a new file and it will be automatically compiled.
# Get all the files needed for the library
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/OneLibrary/*.cpp" "include/OneLibrary/*.h")

# Let's only add 'Backward-CPP' if we are running a debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Adding in Backward-CPP to ${PROJECT_NAME}")
    CPMAddPackage("gh:bombela/backward-cpp#90398eef20f4e7e0e939322d7e84f9c52078a325")
    # packagename_SOURCE_DIR is provided here by CPM (I suppose it uses FetchPackage under the hood which actually gives us the definitions for the packages, similar to NuGet in C# :thumbs_up: )
    # list(APPEND SOURCE_FILES ${backward_SOURCE_DIR}/backward.cpp)
    list(APPEND SOURCE_FILES ${BACKWARD_ENABLE})
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
    add_backward(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
endif()

#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> SFML fmt)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
#target_link_libraries(${PROJECT_NAME} PUBLIC sfml-network fmt::fmt-header-only)

file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS "include/*.h")
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "Debug")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Check inter-procedural optimisation
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    message(STATUS "Inter-procedural Optimisation Enabled")
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Add final compiler options
if(CMAKE_HOST_WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OS_WINDOWS=1 WINVER=0x0A00 _WIN32_WINNT=0x0A00)
    target_compile_options(${PROJECT_NAME} PUBLIC "/MP")
elseif(CMAKE_HOST_APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OS_APPLE=1)
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wfatal-errors")
elseif(CMAKE_HOST_LINUX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OS_LINUX=1)
    target_compile_options(${PROJECT_NAME} PUBLIC "-pthread" "-Wfatal-errors")
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC OS_ERROR=1)
endif()

#install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION include)

if(PACKAGE_TESTS)
    # enable_testing() needs to be here.
    # Reasoning: https://stackoverflow.com/questions/64099136/visual-studio-2019-test-explorer-did-not-find-c-google-tests
    message(STATUS "Adding tests")
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()