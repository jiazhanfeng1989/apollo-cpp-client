# nlohmann_json usage

[nlohmann_json](https://github.com/nlohmann/json) is a popular C++ header only JSON library.

## nlohmann_json installation:
```bash
1. git clone --depth 1 -b v3.12.0 https://github.com/nlohmann/json.git nlohmann_json_3_12_0
2. cd nlohmann_json_3_12_0
3. mkdir build && cd build
4. cmake -DCMAKE_INSTALL_PREFIX=${HOME}/nlohmann_json_3_12_0_custom ..
5. make -j4
6. make install
```
## nlohmann_json in CMakeLists.txt:
```cmake
find_package(nlohmann_json 3.12 QUIET)
if(NOT nlohmann_json_FOUND)
    # If not found, set custom paths and try again
    message(STATUS "nlohmann_json cmake not found in default path, try to use custom nlohmann_json cmake path")
    message(STATUS "nlohmann_json custom cmake path: $ENV{HOME}/nlohmann_json_3_12_0_custom/share/cmake/nlohmann_json")
    set(nlohmann_json_DIR "$ENV{HOME}/nlohmann_json_3_12_0_custom/share/cmake/nlohmann_json")
    find_package(nlohmann_json 3.12 REQUIRED)
else()
    message(STATUS "nlohmann_json found in default path")
    find_package(nlohmann_json 3.12 REQUIRED)
endif()
```