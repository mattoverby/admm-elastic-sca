cmake_minimum_required(VERSION 2.6)
project(admm-elastic-samples C CXX)

#
#	Build Mode
#
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
set( CMAKE_BUILD_TYPE Release )

file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output" )
file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/samples" )
add_definitions( -DOUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/output" ) # OUTPUT_DIR
add_definitions( -DSRC_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}" ) # SRC_ROOT_DIR
option(MCL_BUILD_SAMPLES "Build mcl library samples" OFF)
option(ADMME_BUILD_SAMPLES "Build admm-elastic samples" OFF)

############################################################
#
#	Libraries
#
############################################################


# admm-elastic, submodule in deps/
set( ADMME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/admm-elastic-sca" )
add_subdirectory( ${ADMME_DIR} )
include_directories( ${ADMME_INCLUDE_DIRS} )

# Eigen library (header only), included in admm-elastic's deps/
set( EIGEN_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/deps/admm-elastic-sca/deps/Eigen3" )
include_directories( SYSTEM ${EIGEN_INCLUDE} )

# CppOptimizationLibrary (header only), also in admm-elastic's deps/
set( CPPMIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/admm-elastic-sca/deps/cppoptlib" )
include_directories( ${CPPMIN_DIR}/include )

# OpenMP
find_package(OpenMP)
if(OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# mclscene, submodule in deps/
# Note that adding this subdirectory sets variables for included mclscene's
# dependencies as well (opengl, glfw3, etc...)
set( MCLSCENE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/mclscene" )
add_subdirectory( ${MCLSCENE_DIR} )
include_directories( ${MCLSCENE_INCLUDE_DIRS} )

# Create a variable containing all of the bindings for mclscene
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )
set( ADMME_SAMPLES_SRCS
	src/SimContext.hpp		src/SimContext.cpp
	src/ForceBuilder.hpp		src/ForceBuilder.cpp
)

# Finally, create the library
add_library( admmelasticsamples ${ADMME_SAMPLES_SRCS} )
link_libraries( admmelasticsamples ${MCLSCENE_LIBRARIES} ${ADMME_LIBRARIES} )


############################################################
#
#	Binaries
#
############################################################

# Change output binary directory to keep samples in a different dir
set( OLD_CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples )

	add_executable( windyflag ${CMAKE_CURRENT_SOURCE_DIR}/samples/windyflag/windyflag.cpp )
	target_link_libraries( windyflag admmelasticsamples )

	add_executable( bunnyexpand ${CMAKE_CURRENT_SOURCE_DIR}/samples/bunnyexpand/bunnyexpand.cpp )
	target_link_libraries( bunnyexpand admmelasticsamples )

	add_executable( plinkopony ${CMAKE_CURRENT_SOURCE_DIR}/samples/plinkopony/plinkopony.cpp )
	target_link_libraries( plinkopony admmelasticsamples )

	add_executable( poordillo ${CMAKE_CURRENT_SOURCE_DIR}/samples/poordillo/poordillo.cpp )
	target_link_libraries( poordillo admmelasticsamples )

# Change output binary directory back
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OLD_CMAKE_RUNTIME_OUTPUT_DIRECTORY} )


