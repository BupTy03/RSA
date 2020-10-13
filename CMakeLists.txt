cmake_minimum_required(VERSION 3.16)
project(RSA CXX)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -W -Wall -Wpedantic")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /Oi /Ot /GL")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
endif()


set(SOURCES
        main.cpp
        algorithms.h
        rsa.cpp rsa.h
        utils.cpp utils.h
        RsaProcessor.cpp RsaProcessor.h
        Task.cpp Task.h)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
add_executable(RSA ${SOURCES})
target_link_libraries(RSA PRIVATE boost_program_options)
