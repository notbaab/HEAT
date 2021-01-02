list(APPEND io_src
    OutputMemoryBitStream.cpp
    InputMemoryBitStream.cpp
    shared.cpp
)

add_library(io SHARED ${io_src})

target_link_libraries(io PUBLIC str_utils)

target_include_directories(io
    PUBLIC
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}
)
