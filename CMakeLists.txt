cmake_minimum_required(VERSION 3.10)
project(SuperCoders_Project02 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

# Include directory for header files
include_directories(include)

# Gather all source files (C++ and C)
file(GLOB SOURCES "src/*.cpp" "src/*.c")

# Define the main executable for the crawler
add_executable(crawler ${SOURCES})

# Enable testing and add the test subdirectory
enable_testing()
add_subdirectory(tests)
