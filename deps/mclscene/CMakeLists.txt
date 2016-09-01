# Copyright 2016 Matthew Overby.
# 
# MCLSCENE Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other materials
#    provided with the distribution.
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OF MINNESOTA, DULUTH OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# By Matt Overby (http://www.mattoverby.net)

cmake_minimum_required(VERSION 3.1)
project(mclscene C CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
add_definitions( -DMCLSCENE_BUILD_DIR="${CMAKE_BINARY_DIR}" ) # root build dir
add_definitions( -DMCLSCENE_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )
set( CMAKE_BUILD_TYPE Release )


#
#	compilation options
#
option(MCL_BUILD_SAMPLES "Build mcl library samples" ON)
option(MCL_BUILD_RENDERING "Build mclscene gui classes" ON)


#
#	dependencies
#

# trimesh, included in deps/
set( TRIMESH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/trimesh2" )
add_subdirectory( ${TRIMESH_DIR} ) # sets TRIMESH_INCLUDE and TRIMESH_SRCS

# tetgen, included in deps/
set( TETGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/tetgen" )
add_subdirectory( ${TETGEN_DIR} ) # sets TETGEN_INCLUDE and TETGEN_SRCS

# Rendering libraries
if(MCL_BUILD_RENDERING)

	find_package(OpenGL 4)
	find_package(GLFW 3)
	find_package(GLEW)

	# soil2, included in deps/
	set( SOIL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/soil2" )
	add_subdirectory( ${SOIL2_DIR} ) # sets SOIL2_INCLUDE and SOIL2_SRCS

endif(MCL_BUILD_RENDERING)

# OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#
#	mclscene library
#

# Set up the library, add gui sources if opengl/glfw are found
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
set( MCLSCENE_SRCS
	include/MCL/SceneManager.hpp	src/SceneManager.cpp
	include/MCL/TetMesh.hpp		src/TetMesh.cpp
	include/MCL/TriangleMesh.hpp	src/TriangleMesh.cpp
	include/MCL/PointCloud.hpp	src/PointCloud.cpp
	include/MCL/RenderGL.hpp	src/RenderGL.cpp
	include/MCL/BVH.hpp		src/BVH.cpp
	include/MCL/Param.hpp
	include/MCL/Object.hpp
	include/MCL/VertexSort.hpp
	include/MCL/Camera.hpp
	include/MCL/Light.hpp
	include/MCL/AABB.hpp
	include/MCL/Shader.hpp
	include/MCL/Material.hpp
	include/MCL/Simulator.hpp
	include/MCL/RayIntersect.hpp
	include/MCL/DefaultBuilders.hpp
)

# Create the library
set( MCLSCENE_LIBRARIES mclscene )
set( MCLSCENE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${TRIMESH_INCLUDE} ${TETGEN_INCLUDE} )

# If we want to build with mclscene rendering support, add the window source files
# and add the libraries/includes to the build variables.
if(MCL_BUILD_RENDERING)
	set( MCLSCENE_SRCS ${MCLSCENE_SRCS} ${SOIL2_SRCS} include/MCL/Application.hpp src/Application.cpp )
	set( MCLSCENE_LIBRARIES ${MCLSCENE_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} )
	set( MCLSCENE_INCLUDE_DIRS ${MCLSCENE_INCLUDE_DIRS} ${SOIL2_INCLUDE} ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GL3W_INCLUDE_DIR} )
endif(MCL_BUILD_RENDERING)

# Finally, include our needed files and add the library
include_directories( ${MCLSCENE_INCLUDE_DIRS} )
add_library( mclscene ${TRIMESH_SRCS} ${TETGEN_SRCS} ${MCLSCENE_SRCS} )

# Set the libraries variable in parent scope if exists
get_directory_property(HasParent PARENT_DIRECTORY)
if(HasParent)
	set( MCLSCENE_LIBRARIES ${MCLSCENE_LIBRARIES} PARENT_SCOPE )
	set( MCLSCENE_INCLUDE_DIRS ${MCLSCENE_INCLUDE_DIRS} PARENT_SCOPE )
endif(HasParent)

#
#	samples
#

if(MCL_BUILD_SAMPLES)

	# sample loader
	add_executable( load_test samples/LoadTest.cpp )
	target_link_libraries( load_test ${MCLSCENE_LIBRARIES} )

	add_executable( testbvh samples/BVHTest.cpp )
	target_link_libraries( testbvh ${MCLSCENE_LIBRARIES} )

	# viewer samples
	if(MCL_BUILD_RENDERING)

		add_definitions( ${OpenGL_DEFINITIONS} )
		add_executable( mesh_viewer samples/MeshViewer.cpp )
		target_link_libraries( mesh_viewer mclscene ${MCLSCENE_LIBRARIES} )
		add_executable( bvh_viewer samples/BVHViewer.cpp )
		target_link_libraries( bvh_viewer ${MCLSCENE_LIBRARIES} )

	endif(MCL_BUILD_RENDERING)

endif(MCL_BUILD_SAMPLES)
