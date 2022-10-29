cmake_minimum_required(VERSION 3.22)
project(MultiBus-Examples-C C)

set(MULTIBUS_ROOT       ${CMAKE_SOURCE_DIR}/../..)
set(MULTIBUS_SRC        ${MULTIBUS_ROOT}/host/c)
set(MULTIBUS_PROTOCOL   ${MULTIBUS_ROOT}/protocol)
set(MULTIBUS_PROTOCOL_C ${MULTIBUS_ROOT}/protocol/c)

set(CMAKE_C_STANDARD 11)

# Generator
list(APPEND CMAKE_MODULE_PATH ${MULTIBUS_PROTOCOL})
include(generator)

# find libev
find_library(LIBEV_LIBRARY NAMES ev)
find_path(LIBEV_INCLUDE_DIR ev.h PATH_SUFFIXES include/ev include)
find_package_handle_standard_args(libev DEFAULT_MSG LIBEV_LIBRARY LIBEV_INCLUDE_DIR)

include_directories(${MULTIBUS_SRC} ${MULTIBUS_PROTOCOL_C} ${CMAKE_CURRENT_BINARY_DIR} ${LIBEV_INCLUDE_DIR})

# create static lib
add_library(multibus STATIC
	${MULTIBUS_SRC}/multibus_serial_posix.c
	${MULTIBUS_SRC}/multibus_serial_posix.h
	${MULTIBUS_PROTOCOL_C}/multibus_transport.c
	${MULTIBUS_PROTOCOL_SRC}
)

# create examples
file(GLOB EXAMPLES_C ${CMAKE_SOURCE_DIR}/*.c)
foreach(EXAMPLE_FILE ${EXAMPLES_C})
	get_filename_component(EXAMPLE ${EXAMPLE_FILE} NAME_WE)
	set (SOURCES_EXAMPLE ${CMAKE_SOURCE_DIR}/${EXAMPLE}.c)
	if (EXAMPLE MATCHES ".*_libev.*")
		if (LIBEV_LIBRARY EQUAL "LIBEV_LIBRARY-NOTFOUND")
			message("example ${EXAMPLE} - skipping as libev not found")
		else()
			message("example ${EXAMPLE} (with libev)")
			add_executable(${EXAMPLE} ${SOURCES_EXAMPLE} )
			target_link_libraries(${EXAMPLE} ${LIBEV_LIBRARY} multibus )
		endif()
	else()
		message("example ${EXAMPLE}")
		add_executable(${EXAMPLE} ${SOURCES_EXAMPLE} )
		target_link_libraries(${EXAMPLE} multibus)
	endif()
endforeach(EXAMPLE_FILE)
