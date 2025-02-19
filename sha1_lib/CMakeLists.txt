cmake_minimum_required(VERSION 3.18)
project("SHA1SUM")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	add_definitions(-D_WIN32)
	add_compile_options("/O2")
else()
   if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		message(WARNING "Assuming system in Linux")
	endif()
	add_definitions(-D_LINUX)
	add_compile_options("-O3")
endif()

option(BUILD_SHARED_LIBS "Build the static library" OFF)

if(BUILD_SHARED_LIBS)
	set(LIB_TYPE "SHARED")
	message(WARNING "Building shared library.")	
else()
	set(LIB_TYPE "STATIC")
endif()

message(WARNING "Building library ${LIB_TYPE}")	
#---------------------------------------------------------------------

set(SHA1_LIB "sha1sum")

add_library(
	"${SHA1_LIB}"
	"${LIB_TYPE}"
	sha1.cpp
)

target_include_directories(
	"${SHA1_LIB}"
	PUBLIC	
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(
	"${SHA1_LIB}"
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
)

#---------------------------------------------------------------------
