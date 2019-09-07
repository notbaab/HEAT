
list(APPEND holistic_src
    SetupNetworking.cpp
    SetupManagers.cpp
    SetupWorld.cpp
)

add_library(holistic STATIC ${holistic_src})
# add_library(holistic SHARED ${holistic_src})

target_include_directories(holistic
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)

# not linking with anything since
target_link_libraries(holistic gameobjects packets)
