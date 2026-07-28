cmake_minimum_required(VERSION 3.10)
project(SuperCoders_Project02 CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

# Include directory for header files
include_directories(include include/common include/crawler include/indexer)

# Gather source files by module
file(GLOB COMMON_SOURCES "src/common/*.cpp" "src/common/*.c")
file(GLOB CRAWLER_SOURCES "src/crawler/*.cpp")
file(GLOB INDEXER_SOURCES "src/indexer/*.cpp")

# Define the main executable for the crawler
add_executable(crawler ${COMMON_SOURCES} ${CRAWLER_SOURCES})
target_link_libraries(crawler curl)

# Define the main executable for the indexer
add_executable(indexer ${COMMON_SOURCES} ${INDEXER_SOURCES})

# Enable testing and add the test subdirectory
enable_testing()
add_subdirectory(tests)
