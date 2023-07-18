# OneLibrary - Cross-Platform Input Hooking Library
OneLibrary is a C++ library that allows you to more easily create applications that interact with user input.

# Requirements
This project supports CLion and Visual Studio as the IDEs of choice.

For CLion, some CMake profiles have been shared in this repository. These include:
```
Windows-Clang-Release   - Windows -> Linux      (Cross-Compile)
Windows-Clang-Debug     - Windows -> Linux      (Cross-Compile)
Windows-MSVC-Release    - Windows -> Windows    (Native)
Windows-MSVC-Debug      - Windows -> Windows    (Native)
Windows-MinGW-Release   - Windows -> Linux      (Cross-Compile)
Windows-MinGW-Debug     - Windows -> Linux      (Cross-Compile)
Windows-GCC-Release     - Windows -> Linux      (Cross-Compile)
Windows-GCC-Debug       - Windows -> Linux      (Cross-Compile)
```

On Windows, these have their own requirements which can be installed using [Scoop](https://scoop.sh).
For example, to install GCC and Clang on Windows using `scoop`, you can do:
```shell
scoop install mingw-winlibs-llvm-ucrt
```
To read more about `mingw-winlibs-llvm-ucrt`, see [WinLibs - MSVCRT or UCRT runtime library?](https://winlibs.com)

# How To Build
See [BUILDING](BUILDING.MD).

# Examples
See [examples](docs/examples).
TODO: Add examples here as well as the docs.

# How To Use
- Using [CPM](https://github.com/cpm-cmake/CPM.cmake) by adding `CPMAddPackage("gh:szymonj99/OneLibrary#{COMMIT_HASH_HERE}")`
- Clone this repository and use it as a submodule
- Your own way of adding a C++ library of choice

# License
See the [license file](LICENSE) (subject to change)

# Project Structure
```
./inlude - The header files
./src    - The source files
./tests  - The test files
```

# Get Started

## Linux
```bash
git clone https://github.com/szymonj99/OneLibrary
cd OneLibrary
git submodule --init --recursive
cd vcpkg && ./bootstrap-vcpkg.sh -disableMetrics
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

##Windows
```shell
git clone https://github.com/szymonj99/OneLibrary
cd OneLibrary
git submodule --init --recursive
cd vcpkg && bootstrap-vcpkg.bat -disableMetrics
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

# CMake Files Style
- Use `TRUE` and `FALSE` rather than `ON` or `OFF`

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