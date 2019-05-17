list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

list(APPEND controls_src
    InputManager.cpp
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_library(controls SHARED ${controls_src})
target_include_directories(controls
        PUBLIC
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
)

target_link_libraries(controls ${SDL2_LIBRARY})
