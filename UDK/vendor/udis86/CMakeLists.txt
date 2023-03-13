cmake_minimum_required(VERSION 3.14.4)
project(udis86)

set (CMAKE_C_STANDARD 11)

file(GLOB udis_lib_sources libudis86/*.c libudis86/*.h)
add_library(udis STATIC ${udis_lib_sources})

target_include_directories(
	udis
	PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR}/libudis86
	PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libudis86>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
