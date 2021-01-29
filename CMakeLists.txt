cmake_minimum_required(VERSION 3.10)
project(HEAT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wno-multichar")
if(NOT APPLE)
        # apple includes -fPIC flag by default, need to add it if we aren't on
        # a mac
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_subdirectory(str_utils)
add_subdirectory(logger)
add_subdirectory(datastructures)
add_subdirectory(sockets)
add_subdirectory(IO)
add_subdirectory(packets)
add_subdirectory(events)
# add_subdirectory(messages)
add_subdirectory(managers)
add_subdirectory(math)
add_subdirectory(graphics)
add_subdirectory(controls)
add_subdirectory(gameobjects)
add_subdirectory(engine)
add_subdirectory(holistic)
add_subdirectory(debugging_tools)
add_subdirectory(console)
add_subdirectory(dvr)

# Testing
add_subdirectory(lib/catch)
add_subdirectory(tests)

add_executable(server server.cpp)
add_executable(client client.cpp)

# Copy the image assets
file(COPY images DESTINATION ${CMAKE_BINARY_DIR})


target_include_directories(client
        PUBLIC
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/math
        lib
)

set(shared_link_libs
        engine
        gameobjects
        holistic
        io
        logger
        managers
        math
        # messages
        sockets
        packets
        events
        debugging_tools
        dvr
)

set(client_link_libs
        controls
        graphics
)

target_link_libraries(client PUBLIC ${shared_link_libs} ${client_link_libs})

target_link_libraries(server ${shared_link_libs})
