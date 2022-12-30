# OneLibrary - Cross-Platform Input Hooking Library
OneLibrary is a C++ library that allows you to more easily create applications that interact with user input.

# Examples
See [examples](examples).

# How To Use
- Using [CPM](https://github.com/cpm-cmake/CPM.cmake) by adding `CPMAddPackage("gh:szymonj99/OneLibrary#{COMMIT_HASH_HERE}")`
- Your own way of adding a C++ library of choice

# License
See the [license file](LICENSE) (subject to change)

# Project Structure
```
./src   - The source files for the library
./tests - The test files
```

# Get Started - CMake Build
```shell
git clone https://github.com/szymonj99/OneLibrary
cd OneLibrary
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

# CMake Files Style
- Use `TRUE` and `FALSE` rather than `ON` or `OFF`

# Argument Parsing
Read more about which dependency was chosen [here](docs/arg_parsing.md)

# Testing Framework
Read more about which dependency was chosen [here](docs/testing_frameworks.md)

# Projects Using This Library
- To add your project to this list, [create a PR!](https://github.com/szymonj99/OneLibrary/compare) 👍
