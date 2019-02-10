list(APPEND message_src
    MessageSerializer.cpp
)

add_library(messages SHARED ${message_src})
target_include_directories(messages PUBLIC ${CMAKE_SOURCE_DIR}/IO)
target_link_libraries(messages io)
