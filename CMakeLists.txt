cmake_minimum_required(VERSION 3.6)
project(underscorepp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp underscore.hpp)
add_executable(underscorepp ${SOURCE_FILES})