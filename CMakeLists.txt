cmake_minimum_required(VERSION 3.17)
project(RSA CXX)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wpedantic")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /Oi /Ot /GL")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
endif()

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})

include_directories("D:/Libs/boost_1_69_0")
add_executable(RSA main.cpp algorithms.h)
