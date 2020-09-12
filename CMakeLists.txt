cmake_minimum_required(VERSION 3.17)
project(RSA)

set(CMAKE_CXX_STANDARD 17)

include_directories("D:/Libs/boost_1_69_0")
add_executable(RSA main.cpp algorithms.h)