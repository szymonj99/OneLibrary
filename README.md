# OneLibrary - Cross-Platform Input Hooking Library
OneLibrary is a C++ library that allows you to more easily create applications that interact with user input.

# Get Started

## Windows
Windows is supported, and there are CMake profiles provided for MSVC, GCC, Clang and ClangCL

### Scoop
Management of dependencies is recommended to be done with Scoop.
Scoop provides `versions/mingw-winlibs-llvm` and `versions/mingw-winlibs-llvm-ucrt` (untested), which has been used for development.
However, with a default install, CMake will fail when trying to configure the ClangCL profile.
That is because the executable `mt.exe` will potentially be missing. To solve this, an easy option is to:
- Get the directory of `llvm-mt.exe`: `which llvm-mt.exe` or `which llvm-mt`
- Copy `llvm-mt` to `mt` in the same directory: `cp {LLVM_MT_DIRECTORY}/llvm-mt.exe {LLVM_MT_DIRECTORY}/mv.exe` (replace {LLVM_MT_DIRECTORY} with the output from the previous command).

OR:

Set `CMAKE_MT` to `llvm-mt.exe` explicitly.
For example, this can be done by calling `cmake {ARGS} -D CMAKE_MT=/path/to/llvm-mt.exe` for ClangCL _only_,
or in `CMakeLists.txt` before calling `project()`.

When using ClangCL and `sccache`, you may encounter an error that `file /wd4668 can't be found`. That issue is documented here: https://github.com/mozilla/sccache/issues/1843

Instead, `buildcache` can be used.

## Linux
Linux is supported as long as you have installed `libevdev` or the `evdev` library.
More work on supporting more compilers on Linux is being done.
## macOS
macOS is not currently supported.
## Others
Other operating systems (host or targets) are not currently supported.
## Cross-Compiling
Cross-compiling is not currently supported.

# Examples
See [examples](docs/examples).
TODO: Add examples here as well as the docs.

# How To Use
- Using [CPM](https://github.com/cpm-cmake/CPM.cmake) by adding `CPMAddPackage("gh:szymonj99/OneLibrary#{COMMIT_HASH_HERE}")`
- Your own way of adding a C++ library of choice

# License
See the [license file](LICENSE) (subject to change)

# Project Structure
```
./inlude - The header files
./src    - The source files
./tests  - The test files
```

# Get Started - CMake Build
```shell
git clone https://github.com/szymonj99/OneLibrary
cd OneLibrary
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

# Testing Framework
Read more about which dependency was chosen [here](docs/testing_frameworks.md)

# Projects Using This Library
- [OneControl](https://github.com/szymonj99/OneControl)
- To add your project to this list, [create a PR!](https://github.com/szymonj99/OneLibrary/compare) 👍

# Developing With Local Changes
If you want to make changes to this library, you can test these changes locally.
For example, if you have a project that uses this as a dependency through [CPM](https://github.com/cpm-cmake/CPM.cmake), you can instead do the following:
1. `git clone git@github.com:szymonj99/OneLibrary.git` one level up from your current project
2. Add `CPMAddPackage(NAME OneLibrary PATHS ../OneLibrary)` to your `CMakeLists.txt` to add in the dependency from local files.
For further information, see [this](https://github.com/szymonj99/OneControl/blob/main/CMakeLists.txt).

# TODO
- Backward-cpp has been removed for now;
  - Configuring returns an error with Backward enabled; CMake Deprecation Warning.

## CMakePresets.json
- Currently supported generator is `Ninja`.
- MSVC generator is not included by default.

## Flags
Compiler flags and configs/setup needed for this project should be performed in CMakeLists.txt.
This will result in the necessary configs being abstracted from a user of this library, allowing it to be added to someone's project without any more configs needed to be done.
Unicode support should be added in a way that supports both unicode and non-unicode preprocessor defines.
CMakeLists.txt will include necessary flags such as `/EHsc` when compiling with MSVC or ClangCL (or was it Clang?).
`UNICODE` will instead be defined by the application that uses OneLibrary.
