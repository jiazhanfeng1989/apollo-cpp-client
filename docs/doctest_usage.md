# doctest usage
[doctest](https://github.com/doctest/doctest) is a new C++ header only testing framework.

## doctest installation:
```bash 
1:git clone --depth 1 -b v2.4.12 https://github.com/doctest/doctest.git doctest_2_4_12
2:cd doctest_2_4_12
3:mkdir build && cd build
4:cmake -DCMAKE_INSTALL_PREFIX=${HOME}/doctest_2_4_12_custom ..
5:make -j4 && make install
```

## doctest in CMakeLists.txt:
```cmake
find_package(doctest 2.4 QUIET) 
if(NOT doctest_FOUND)
    # If not found, set custom paths and try again
    message(STATUS "doctest cmake not found in default path, try to use custom doctest cmake path")
    message(STATUS "doctest custom cmake path: $ENV{HOME}/doctest_2_4_12_custom/share/cmake/doctest")
    set(doctest_DIR "$ENV{HOME}/doctest_2_4_12_custom/lib/cmake/doctest")
    find_package(doctest 2.4 REQUIRED)
else()
    message(STATUS "doctest found in default path")
    find_package(doctest 2.4 REQUIRED)
endif()

target_link_libraries(${TARGET} doctest::doctest)
```