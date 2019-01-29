list(APPEND io_src
    OutputMemoryBitStream.cpp
    InputMemoryBitStream.cpp
    shared.cpp
)

add_library(io SHARED ${io_src})
