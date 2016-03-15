project(microcore)
cmake_minimum_required(VERSION 2.8.11)

# Options
option(ENABLE_TESTS "Enable tests and coverage" OFF)

# Configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

if(ENABLE_TESTS)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        message("Building with tests and coverage")
        set(CMAKE_CXX_COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage -lgcov")
    endif(CMAKE_BUILD_TYPE MATCHES Debug)
endif(ENABLE_TESTS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Projects
enable_testing()
add_subdirectory(src/3rdparty)
add_subdirectory(src/tools)
add_subdirectory(src/lib)
if(ENABLE_TESTS)
    add_subdirectory(src/tests)
endif(ENABLE_TESTS)
 
