cmake_minimum_required(VERSION 3.12)
project(HEAT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wno-multichar")

add_subdirectory(networking)
add_subdirectory(IO)
add_subdirectory(packets)
add_subdirectory(messages)
add_subdirectory(tests)
add_subdirectory(lib/catch)

add_executable(main main.cpp)

target_link_libraries(main networking io packets)
