# TODO: This shouldn't really be called by the library CMakeLists.txt, but instead by an end-project that wants to use it.
function(set_buildcache_if_found)
    # ccache isn't necessary here as sccache is an improvement over it.
    # We don't use sccache as it has an issue on ClangCL on Windows: https://github.com/mozilla/sccache/issues/1843
    #find_program(COMPILER_CACHE_PROGRAM sccache)
    find_program(COMPILER_CACHE_PROGRAM buildcache)
    if(COMPILER_CACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${COMPILER_CACHE_PROGRAM})
    endif()
endfunction()

function(set_ipo PROJECT)
    # Check inter-procedural optimisation
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        # https://en.wikipedia.org/wiki/Interprocedural_optimization
        # We don't have `-flto` flags for Clang and GCC as this provides a platform-independent option for those.
        # This also takes care of the `/GL` compiler flag and `/LTCG` linker flag when using MSVC.
        set_target_properties(${PROJECT} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endfunction()

function(set_threads PROJECT)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT} PUBLIC Threads::Threads)
    unset(CMAKE_THREAD_PREFER_PTHREAD) # Let's make this idempotent
endfunction()

function(set_install PROJECT)
    # Taken from: https://github.com/CharmedBaryon/CommonLibSSE-NG/blob/main/CMakeLists.txt
    install(
            TARGETS "${PROJECT}"
            EXPORT "${PROJECT}-targets"
    )

    install(
            EXPORT "${PROJECT}-targets"
            NAMESPACE "${PROJECT}::"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT}"
    )

    configure_file(
            cmake/config.cmake.in
            "${PROJECT}Config.cmake"
            @ONLY
    )

    install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}Config.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT}"
    )

    install(TARGETS ${PROJECT} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT})
endfunction()

# TODO: Continue working on this.
function(set_compilers PROJECT)
    # https://stackoverflow.com/questions/49480535/how-to-differentiate-between-clang-and-clang-cl
    # Note that recent versions of CMake suggest to use CMAKE_<LANG>_COMPILER_FRONTEND_VARIANT instead of SIMULATE_ID for this purpose.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${PROJECT} PUBLIC "/EHsc")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CXX_FLAG_PREFIX "-")
        target_compile_options(${PROJECT} PUBLIC "-fexceptions")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
            # Clang
            target_compile_options(${PROJECT} PUBLIC "-fexceptions")
        elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            # ClangCL
            set(CXX_FLAG_PREFIX "/")
            target_compile_options(${PROJECT} PUBLIC "/EHsc")
        else()
            message(WARNING "Unknown Clang compiler frontend")
        endif()
    else()
        message(WARNING "Unknown C++/CXX compiler")
    endif()

    # TODO: Check target platform here
    #message(STATUS ${CMAKE_SYSTEM_NAME}) # TARGET SYSTEM NAME
    #message(STATUS ${CMAKE_HOST_SYSTEM_NAME})

    # Cross compiling here doesn't really matter I think.
    # We need to always check CMAKE_SYSTEM_NAME, as that is the target.
    # Compiler flags depend on the GNU/MSVC style, not the host/target system.

    # TODO: Here we will set options that are abstracted from the end-user;
    # The end-user doesn't care about there, and shouldn't be hassled with having to configure them correctly in their own
    # CMakeLists.txt or CMakePresets.json when adding OneLibrary as a dependency.

    # TODO: Should these be PRIVATE or PUBLIC? :thinking:
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(${PROJECT} PRIVATE OS_WINDOWS)
        target_compile_definitions(${PROJECT} PRIVATE WIN32_LEAN_AND_MEAN NO_MIN_MAX NOMINMAX)
        # TODO: Might be worth letting users change this?
        target_compile_definitions(${PROJECT} PRIVATE WINVER=0x0A00) # Windows 10
        target_compile_definitions(${PROJECT} PRIVATE WIN32_WINNT=0x0A00) # Windows 10
        target_compile_definitions(${PROJECT} PRIVATE STRICT) # https://learn.microsoft.com/en-us/windows/win32/winprog/enabling-strict
        # No UNICODE defines here; I'm letting end-users decide on if they want unicode or not;
        # Although, providing a unicode-only version that doesn't work when non-unicode might be nice?
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(${PROJECT} PRIVATE OS_LINUX)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin") # Untested, might not be correct.
        # TODO: Not much point in giving the user a warning if we define OS_ERROR; the project won't compile.
        target_compile_definitions(${PROJECT} PRIVATE OS_APPLE OS_ERROR)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Generic") # Untested, might not be correct.
        # TODO: Not much point in giving the user a warning if we define OS_ERROR; the project won't compile.
        target_compile_definitions(${PROJECT} PRIVATE OS_ERROR)
    else()
        # TODO: Not much point in giving the user a warning if we define OS_ERROR; the project won't compile.
        message(WARNING "Unknown TARGET system name")
        target_compile_definitions(${PROJECT} PRIVATE OS_ERROR)
    endif()
endfunction()
