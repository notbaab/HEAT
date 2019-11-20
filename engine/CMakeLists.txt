# Basically the largest point of entry, it takes all the modules
# and glues them together into something that resembles a game engine

list(APPEND engine_src
    Engine.cpp
)

add_library(engine SHARED ${engine_src})
