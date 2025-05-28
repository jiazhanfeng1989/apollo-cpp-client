# Apollo C++ Client

# Overview
A C++ Client library for the [Apollo](https://github.com/apolloconfig/apollo) configuration center.

## Features
- Support for Apollo long polling and notification mechanisms
- Support for Apollo configuration updates
- Support for Apollo gray release by label
- Thread-safe access to configuration values

## TODO Features
- Implement HTTPS support.

## Prerequisites
- C++14 compatible compiler
- CMake 3.11 or higher
- Boost 1.84 or higher
- nlohmann_json 3.11.2 or higher
- doctest 2.4 or higher (for testing, optional)
- Doxygen 1.14.0 or higher (for documentation generation, optional)

## Building
```bash
mkdir build && cd build

# This flag generates a `compile_commands.json` file for integration with tools like clangd.
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# Use `make -jN` to speed up the build process, where `N` is the number of parallel jobs.
make -j4

# Now build the docs target, which generates the documentation.
# This requires Doxygen version 1.14.0 to be installed on your system.
# If you don't have Doxygen installed, you can skip this step.
# ref: https://github.com/doxygen/doxygen
# docs output will be built in the build/docs directory
cmake --build . --target docs
```

## Usage
```c++
```

# 💡 Tips
- [boost usage](./docs/boost_usage.md) provides detailed instructions on how to use Boost libraries with this project.
- [nlohmann_json](./docs/nlohmann_json_usage.md) provides detailed instructions on how to use nlohmann_json with this project.
- [doctest usage](./docs/doctest_usage.md) provides detailed instructions on how to use doctest for unit testing in this project.
