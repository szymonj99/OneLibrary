set(TESTS_MANUAL_NAME OneLibraryTestsManual)

add_executable(${TESTS_MANUAL_NAME} KeyboardTests.cpp MouseTests.cpp)

target_link_libraries(${TESTS_MANUAL_NAME} PRIVATE OneLibrary Catch2::Catch2WithMain)

target_include_directories(${TESTS_MANUAL_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_features(${TESTS_MANUAL_NAME} PRIVATE cxx_std_20)

set_target_properties(${TESTS_MANUAL_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${TESTS_MANUAL_NAME} PROPERTIES CMAKE_CXX_STANDARD 20)
set_target_properties(${TESTS_MANUAL_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED TRUE)
set_target_properties(${TESTS_MANUAL_NAME} PROPERTIES UNICODE TRUE)
set_target_properties(${TESTS_MANUAL_NAME} PROPERTIES CMAKE_BUILD_PARALLEL_LEVEL 8)
set_target_properties(${TESTS_MANUAL_NAME} PROPERTIES USE_FOLDERS TRUE)

if(CMAKE_HOST_WIN32)
    target_compile_definitions(${TESTS_MANUAL_NAME} PRIVATE OS_WINDOWS=1 WINVER=0x0A00 _WIN32_WINNT=0x0A00 _UNICODE=1 UNICODE=1 WIN32_LEAN_AND_MEAN=1 NOMINMAX=1)
    #target_compile_options(${TESTS_MANUAL_NAME} PRIVATE "/MP")
elseif(CMAKE_HOST_APPLE)
    target_compile_definitions(${TESTS_MANUAL_NAME} PRIVATE OS_APPLE=1 _UNICODE=1 UNICODE=1)
    target_compile_options(${TESTS_MANUAL_NAME} PRIVATE "-Wfatal-errors")
elseif(CMAKE_HOST_LINUX)
    target_compile_definitions(${TESTS_MANUAL_NAME} PRIVATE OS_LINUX=1 _UNICODE=1 UNICODE=1)
    target_compile_options(${TESTS_MANUAL_NAME} PRIVATE "-pthread" "-Wfatal-errors")
else()
    target_compile_definitions(${TESTS_MANUAL_NAME} PRIVATE OS_ERROR=1)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${TESTS_MANUAL_NAME})
