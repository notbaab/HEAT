cmake_minimum_required(VERSION 3.10)
project(HEAT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wno-multichar")

add_subdirectory(networking)
add_subdirectory(IO)
add_subdirectory(packets)
add_subdirectory(messages)
add_subdirectory(managers)
add_subdirectory(math)
add_subdirectory(graphics)
add_subdirectory(controls)
add_subdirectory(gameobjects)

add_subdirectory(lib/catch)
add_subdirectory(tests)

add_executable(main main.cpp)

add_executable(client client.cpp)
target_include_directories(client
        PUBLIC
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/math
        lib
)

target_link_libraries(client math graphics controls gameobjects)

target_link_libraries(main networking io packets)
