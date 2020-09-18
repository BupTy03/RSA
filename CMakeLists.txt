cmake_minimum_required(VERSION 3.17)
project(RSA CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})

include_directories("D:/Libs/boost_1_69_0")
add_executable(RSA main.cpp algorithms.h)