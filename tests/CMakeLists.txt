set(TEST_SOURCES
    main.cpp
    packets.cpp
    managers.cpp
)

add_executable(tests ${TEST_SOURCES})

target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(tests PRIVATE catch io packets messages managers)
