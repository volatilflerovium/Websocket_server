# ------------------------------------------------------------------------------
# Header
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.6)
project(Server-Client)

find_package (Threads)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall")

# ------------------------------------------------------------------------------
# Executable
# ------------------------------------------------------------------------------

include_directories(${CMAKE_BINARY_DIR}/include)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES1
	example_cmds.cpp
	websocket_server.cpp
)

add_executable(example_cmds ${SOURCES1})
target_compile_definitions(example_cmds PUBLIC SERVER=1)
target_link_libraries(example_cmds ${CMAKE_THREAD_LIBS_INIT})


set(SOURCES2
	example_gps.cpp
	websocket_server.cpp
)

add_executable(example_gps ${SOURCES2})
target_compile_definitions(example_gps PUBLIC SERVER=2)
target_link_libraries(example_gps ${CMAKE_THREAD_LIBS_INIT})

set(SOURCES3
	example3.cpp
	websocket_server.cpp
)

add_executable(example3 ${SOURCES3})
target_compile_definitions(example3 PUBLIC SERVER=3)
target_link_libraries(example3 ${CMAKE_THREAD_LIBS_INIT})
