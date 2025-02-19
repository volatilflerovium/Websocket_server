cmake_minimum_required(VERSION 3.18)

project(Websocket_server_lib)

########################################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options("-O3")

find_package(Threads REQUIRED)

add_subdirectory("sha1_lib")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../bin)

option(BUILD_SHARED_LIB "Build the static library" ON)

if(BUILD_SHARED_LIB)
	set(LIB_TYPE SHARED)
else()
	set(LIB_TYPE STATIC)
endif()

set(SHA1_LIB "sha1sum")

#=====================================================================

set(WEBSOCKET_SERVER_LIB websocket_server)

add_library(
	"${WEBSOCKET_SERVER_LIB}"
	"${LIB_TYPE}"
	websocket_server.cpp
)

target_include_directories(
	"${WEBSOCKET_SERVER_LIB}"
	PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/sha1_lib/include" 
)

target_link_libraries(
	"${WEBSOCKET_SERVER_LIB}"
	PRIVATE
	"${SHA1_LIB}"
	Threads::Threads
)

set_target_properties(
	"${WEBSOCKET_SERVER_LIB}"
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

######################################################################
