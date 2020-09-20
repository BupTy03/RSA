cmake_minimum_required(VERSION 3.17)
project(RSA CXX)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wpedantic")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /Oi /Ot /GL")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -static -static-libgcc -static-libstdc++")
    endif()
endif()

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
add_executable(RSA main.cpp algorithms.h rsa.cpp rsa.h utils.cpp utils.h RsaProcessor.cpp RsaProcessor.h Task.cpp Task.h)
target_link_libraries(RSA PRIVATE ${Boost_LIBRARIES})
