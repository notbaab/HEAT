list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

list(APPEND graphics_src
    GraphicsDriver.cpp
    TextureManager.cpp
    WindowManager.cpp
)

add_library(graphics SHARED ${graphics_src})

target_include_directories(graphics
        PUBLIC
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(graphics ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} math)
