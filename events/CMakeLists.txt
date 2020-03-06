list(APPEND events_src
    EventManager.cpp
)

add_library(events SHARED ${events_src}  )

target_link_libraries(events logger)

target_include_directories(events
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)
