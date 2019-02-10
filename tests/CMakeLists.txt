set(TEST_SOURCES
    main.cpp
    packets.cpp
)

add_executable(test ${TEST_SOURCES})

target_include_directories(test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(test PRIVATE catch io packets messages)
