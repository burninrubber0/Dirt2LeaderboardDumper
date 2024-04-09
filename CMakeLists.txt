cmake_minimum_required(VERSION 3.14)
project(Dirt2LbDumper CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
	${SOURCES}
	src/dumper.cpp
	src/main.cpp
	)

set(HEADERS
	${HEADERS}
	include/dumper.h
	)

add_executable(Dirt2LbDumper ${SOURCES} ${HEADERS})

# cppcodec
set(CPPCODEC_ROOT ${ROOT}\\external\\cppcodec)
add_subdirectory(external/cppcodec "${CMAKE_CURRENT_BINARY_DIR}/external/cppcodec" EXCLUDE_FROM_ALL)
target_include_directories(Dirt2LbDumper PRIVATE ${CPPCODEC_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/external/cppcodec")

# cpp-httplib
set(HTTPLIB_ROOT ${ROOT}\\external\\cpp-httplib)
add_subdirectory(external/cpp-httplib "${CMAKE_CURRENT_BINARY_DIR}/external/cpp-httplib" EXCLUDE_FROM_ALL)
target_include_directories(Dirt2LbDumper PRIVATE ${HTTPLIB_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/external/cpp-httplib")

# tinyxml2
set(TINYXML_ROOT ${ROOT}\\external\\tinyxml2)
add_subdirectory(external/tinyxml2 "${CMAKE_CURRENT_BINARY_DIR}/external/tinyxml2" EXCLUDE_FROM_ALL)
target_include_directories(Dirt2LbDumper PRIVATE ${TINYXML_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/external/tinyxml2")

# Dirt2LbDumper includes
target_include_directories(Dirt2LbDumper PRIVATE "${ROOT}/include")

target_link_libraries(Dirt2LbDumper PRIVATE cppcodec httplib tinyxml2)

# VS stuff
set_property(DIRECTORY ${ROOT} PROPERTY VS_STARTUP_PROJECT Dirt2LbDumper)
source_group(TREE ${ROOT} FILES ${SOURCES} ${HEADERS})
