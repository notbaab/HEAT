# Note, does not compile in this source tree. Include it in a wireshark source
# tree build
# link this directory into a w
include(WiresharkPlugin)

# Plugin name and version info (major minor micro extra)
set_module_info(heat 0 0 1 0)
set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 14)
# set the base of our HEAT directory
set(HEAT_DIR "/Users/et/Dropbox/Programming/HEAT/")
# find "-fopenmp"

add_library(packets SHARED IMPORTED)
# You can define two import-locations: one for debug and one for release.
set_target_properties(packets PROPERTIES IMPORTED_LOCATION /Users/et/sublime-build/HEAT/packets/libpackets.dylib)

string(FIND ${CMAKE_CXX_FLAGS} "std=c++11" FLAG_INDEX)
# check if we found it
if (FLAG_INDEX GREATER 0)
   message(STATUS "########## WE'VE GOT AT LEAST ONE!")
   message(STATUS ${CMAKE_CXX_FLAGS})
   string(REPLACE "std=c++11" "std=c++14" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
   message(STATUS ${CMAKE_CXX_FLAGS})
else()
   message(STATUS "########## WE'VE GOT NONE; ADD IT!")
   message(STATUS ${CMAKE_CXX_FLAGS})
   # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

set(DISSECTOR_SRC
    packet-heat.cpp
)

set(PLUGIN_FILES
    plugin.c
    ${DISSECTOR_SRC}
)

set_source_files_properties(
    ${PLUGIN_FILES}
    PROPERTIES
    COMPILE_FLAGS "${WERROR_COMMON_FLAGS}"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HEAT_DIR}
)

register_plugin_files(plugin.c
    plugin
    ${DISSECTOR_SRC}
)

add_plugin_library(heat epan)

target_link_libraries(heat packets epan)

install_plugin(heat epan)

file(GLOB DISSECTOR_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
CHECKAPI(
    NAME
      heat
    SWITCHES
      -g abort -g termoutput -build
    SOURCES
      ${DISSECTOR_SRC}
      ${DISSECTOR_HEADERS}
)

#
# Editor modelines  -  http://www.wireshark.org/tools/modelines.html
#
# Local variables:
# c-basic-offset: 8
# tab-width: 8
# indent-tabs-mode: t
# End:
#
# vi: set shiftwidth=8 tabstop=8 noexpandtab:
# :indentSize=8:tabSize=8:noTabs=false:
#
