cmake_minimum_required(VERSION 2.6)

# Build as a library
add_definitions( -DTETLIBRARY )

set( TETGEN_INCLUDE
	${CMAKE_CURRENT_SOURCE_DIR}
PARENT_SCOPE )

set( TETGEN_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/tetgen.h
	${CMAKE_CURRENT_SOURCE_DIR}/tetgen.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/predicates.cxx
PARENT_SCOPE )
