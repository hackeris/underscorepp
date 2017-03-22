cmake_minimum_required(VERSION 3.6)
project(underscorepp)

set(CMAKE_CXX_STANDARD 11)
include_directories(src)

set(SOURCE_FILES test/test.cpp src/underscore.hpp)
add_executable(underscorepp ${SOURCE_FILES})
target_link_libraries(underscorepp pthread)