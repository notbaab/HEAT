add_library(messages INTERFACE)
target_include_directories(messages INTERFACE ${CMAKE_SOURCE_DIR}/IO ${CMAKE_SOURCE_DIR}/packets)
# target_link_libraries(messages io packets)

