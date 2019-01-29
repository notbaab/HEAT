cmake_minimum_required(VERSION 3.13)
project(HEAT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wno-multichar")

add_subdirectory(networking)
add_subdirectory(IO)

add_executable(test main.cpp)
target_link_libraries(test networking io)
