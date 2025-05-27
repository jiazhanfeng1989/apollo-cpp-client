# Boost Usage
Boost `b2` installation not support cmake target model.<br>
- If Boost is built with `cmake`, you can use `target_link_libraries(${TARGET} PRIVATE Boost::url Boost::asio)` to link boost libraries.
- If Boost is built with `b2`, you can use `include_directories(${Boost_INCLUDE_DIRS}) and link_directories(${Boost_LIBRARY_DIRS})` to set boost installation.

## Boost `cmake` installation:
```bash
1. git clone --depth 1 -b boost-1.84.0 https://github.com/boostorg/boost.git boost_1_84_0
2. cd boost_1_84_0
3. cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/boost_1_84_0_custom -DBOOST_EXCLUDE_LIBRARIES="test" -DBOOST_ENABLE_CMAKE=ON -DCMAKE_BUILD_TYPE=Release
4. cmake --build build --target install -j4
```

## Boost `cmake` installation integrate in CMakeLists.txt:
```cmake
find_package(Boost 1.84 QUIET)
if(NOT Boost_FOUND)
   # If Boost not found, set custom paths and try again
   set(BOOST_ROOT "$ENV{HOME}/boost_1_84_0_custom")
   find_package(Boost 1.84 REQUIRED COMPONENTS url asio)
else()
   message(STATUS "Boost found in default path")
   find_package(Boost 1.84 REQUIRED COMPONENTS url asio)
endif()
...
...
...
target_link_libraries(${TARGET} PRIVATE Boost::url Boost::asio)
```

## Boost `b2` installation:
```bash
1. git clone --depth 1 -b boost-1.84.0 https://github.com/boostorg/boost.git boost_1_84_0
2. cd boost_1_84_0
3. ./bootstrap.sh
4. ./b2 install --prefix=${HOME}/boost_1_84_0_custom --without-thread --without-test -j4
```

## Boost `b2` installation integrate in CMakeLists.txt:
```cmake
find_package(Boost 1.84 QUIET)
if(NOT Boost_FOUND)
    # If Boost not found, set custom paths and try again
    message(STATUS "Boost not found in default path, try to use custom Boost path")
    message(STATUS "Boost custom path: $ENV{HOME}/boost_1_84_0_custom")
    set(Boost_INCLUDE_DIRS "$ENV{HOME}/boost_1_84_0_custom/include")
    set(Boost_LIBRARY_DIRS "$ENV{HOME}/boost_1_84_0_custom/lib")

    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(STATUS "Boost found in default path")
    find_package(Boost 1.84 REQUIRED)
endif()
...
...
...
target_link_libraries(${TARGET} PRIVATE boost_url)
```