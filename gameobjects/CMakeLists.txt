
list(APPEND gameobject_src
    SimpleGameObject.cpp
    Player.cpp
    PlayerClient.cpp
    PlayerServer.cpp
    Registry.cpp
    World.cpp
    WorldClient.cpp
)

add_library(gameobjects SHARED ${gameobject_src})

target_include_directories(gameobjects
        PUBLIC
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(gameobjects graphics ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} math events logger io)
