
list(APPEND gameobject_src
    SimpleGameObject.cpp
    Registry.cpp
    World.cpp
)

add_library(gameobjects SHARED ${gameobject_src})

target_include_directories(gameobjects
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(gameobjects math)
