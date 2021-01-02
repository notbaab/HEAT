set(TEST_SOURCES
    main.cpp
    bit-packets.cpp
    managers.cpp
    json-packets.cpp
)

add_executable(tests ${TEST_SOURCES})

target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(tests PRIVATE catch io packets managers holistic)
