cmake_minimum_required(VERSION 3.6)

project(Websocket_client_examples)

find_package(Threads REQUIRED)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall")

add_compile_options("-O3")

find_library(
	WEBSOCKET_SERVER_LIB
	NAMES websocket_server
	PATHS "${CMAKE_SOURCE_DIR}/../lib" 
	REQUIRED
)

# ------------------------------------------------------------------------------
# Executables
# ------------------------------------------------------------------------------

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_executable(
	example_cmds
	example_cmds.cpp
)	

target_compile_definitions(
	example_cmds
	PUBLIC
	SERVER=1
)

target_link_libraries(
	example_cmds
	PRIVATE
	"${WEBSOCKET_SERVER_LIB}"
	Threads::Threads
)

target_include_directories(
	example_cmds
	PUBLIC
	"${CMAKE_SOURCE_DIR}/../include"
)

#---------------------------------------------------------------------

add_executable(
	example_gps
	example_gps.cpp
)

target_compile_definitions(
	example_gps
	PUBLIC
	SERVER=2
	DATA_FILE="${CMAKE_SOURCE_DIR}/Websocket_GPS_path/GPS_data.txt"
)

target_link_libraries(
	example_gps
	PRIVATE
	"${WEBSOCKET_SERVER_LIB}"
	Threads::Threads
)

target_include_directories(
	example_gps
	PUBLIC
	"${CMAKE_SOURCE_DIR}/../include"
)

#---------------------------------------------------------------------

add_executable(example3
	example3.cpp
)

target_compile_definitions(
	example3
	PUBLIC
	SERVER=3
	DATA_FILE="${CMAKE_SOURCE_DIR}/Websocket_GPS_path/GPS_data.txt"
)

target_link_libraries(
	example3
	"${WEBSOCKET_SERVER_LIB}"
	Threads::Threads
)

target_include_directories(
	example3
	PRIVATE
	"${CMAKE_SOURCE_DIR}/../include"
)

#---------------------------------------------------------------------
