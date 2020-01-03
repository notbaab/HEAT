list(APPEND events_src
    # Event.cpp
    EventManager.cpp
)

add_library(events SHARED ${events_src})

target_include_directories(events
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)
