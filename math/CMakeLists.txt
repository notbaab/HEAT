list(APPEND math_src
    Vector3.cpp
)

add_library(math SHARED ${math_src})
