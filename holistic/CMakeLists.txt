
list(APPEND holistic_src
    HNetworkManager.cpp
    SetupSerializers.cpp
)

add_library(holistic STATIC ${holistic_src})
# add_library(holistic SHARED ${holistic_src})

target_include_directories(holistic
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(holistic packets)
