list(APPEND dvr_src
        DVR.cpp
)

add_library(dvr STATIC ${dvr_src})
target_include_directories(dvr PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(dvr events)
