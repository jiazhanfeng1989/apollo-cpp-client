cmake_minimum_required(VERSION 3.11)
project(ApolloCPPClient)
include(cmake/tools.cmake)

set(APOLLO_CLIENT_TARGET apolloclient)

# === C++ Standard ===
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message("CMAKE_CXX_STANDARD: " ${CMAKE_CXX_STANDARD})
message("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
message("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})

option(BUILD_DOCS "Build API documentation" ON)
option(BUILD_DEMO "Build demo application" ON)
option(BUILD_TEST "Build Test" ON)

# add boost library
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

# add nlohmann_json library
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

func_collect_source_files(HEADER_FILES ${PROJECT_SOURCE_DIR}/include)
func_collect_source_files(SRC_FILES ${PROJECT_SOURCE_DIR}/src)
set(ALL_FILES ${HEADER_FILES} ${SRC_FILES})

add_library(${APOLLO_CLIENT_TARGET} STATIC
    ${ALL_FILES}
)

# func_auto_format_code(${APOLLO_CLIENT_TARGET} ${ALL_FILES})
target_include_directories(${APOLLO_CLIENT_TARGET}
    PUBLIC include
    PRIVATE src
)

target_link_libraries(${APOLLO_CLIENT_TARGET}
    PRIVATE nlohmann_json::nlohmann_json
)

# demo executable
if(BUILD_DEMO)
    add_subdirectory(demo)
else()
    message(STATUS "Skipping demo application build")
endif()

# test executable
if(BUILD_TEST)
    # notes enable_testing() must be called before add_subdirectory(test)
    enable_testing()
    add_subdirectory(test)
else()
    message(STATUS "Skipping test build")
endif()

# Add documentation target
if(BUILD_DOCS)
    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
        message(WARNING "Doxygen not found, documentation will not be generated")
    endif()
else()
    message(STATUS "Skipping documentation build")
endif()
