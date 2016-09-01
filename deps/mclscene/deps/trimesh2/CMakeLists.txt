project(trimesh CXX C)
cmake_minimum_required(VERSION 2.6.2)

#find_package(OpenGL REQUIRED)
#find_package(GLUT REQUIRED)

#if (__APPLE__)
#	add_definitions(-DDARWIN)
#endif(__APPLE__)

set( TRIMESH_INCLUDE
#	${trimesh_SOURCE_DIR}/include/GL
	${trimesh_SOURCE_DIR}/include
	${trimesh_SOURCE_DIR}
PARENT_SCOPE )

set( SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}/libsrc" )

set( TRIMESH_SRCS
	${SRCDIR}/TriMesh_bounding.cc
	${SRCDIR}/TriMesh_connectivity.cc
	${SRCDIR}/TriMesh_curvature.cc
	${SRCDIR}/TriMesh_io.cc
	${SRCDIR}/TriMesh_grid.cc
	${SRCDIR}/TriMesh_normals.cc
	${SRCDIR}/TriMesh_pointareas.cc
	${SRCDIR}/TriMesh_stats.cc
	${SRCDIR}/TriMesh_tstrips.cc
	${SRCDIR}/GLCamera.cc
	${SRCDIR}/ICP.cc
	${SRCDIR}/KDtree.cc
	${SRCDIR}/conn_comps.cc
	${SRCDIR}/diffuse.cc
	${SRCDIR}/edgeflip.cc
	${SRCDIR}/faceflip.cc
	${SRCDIR}/filter.cc
	${SRCDIR}/lmsmooth.cc
	${SRCDIR}/overlap.cc
	${SRCDIR}/remove.cc
	${SRCDIR}/reorder_verts.cc
	${SRCDIR}/shared.cc
	${SRCDIR}/subdiv.cc
PARENT_SCOPE )
