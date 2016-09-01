// Copyright 2016 Matthew Overby.
// 
// MCLSCENE Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OF MINNESOTA, DULUTH OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// By Matt Overby (http://www.mattoverby.net)

#ifndef MCLSCENE_POINTCLOUD_H
#define MCLSCENE_POINTCLOUD_H 1

#include "Object.hpp"
#include "AABB.hpp"

namespace mcl {

//
//	A point cloud is just a collection of vertices
//	stored in a trimesh sans faces/normals.
//	It's used for fluids, gasses, etc...
//
//	Eventually I will add support for:
//		Ray tracing on its convex hull
//		OGL rendering modes (as points, convex hull, semitransparent spheres, etc...)
//

class PointCloud : public BaseObject {
private: std::shared_ptr<trimesh::TriMesh> data;
public:
	PointCloud( std::string mat="" ) : data(new trimesh::TriMesh), vertices(data->vertices), aabb(new AABB) {}

	// Mesh data
	std::vector<trimesh::point> &vertices;
	std::vector<double> radii;

	// General getters
	std::string get_type() const { return "pointcloud"; }
	const std::shared_ptr<trimesh::TriMesh> get_TriMesh(){ return data; }
	std::string get_material() const { return material; }
	void set_material( std::string mat ){ material=mat; }
	std::string get_xml( std::string obj_name, int mode=0 );
	void bounds( trimesh::vec &bmin, trimesh::vec &bmax );
	void update(){ aabb->valid=false; }

	void apply_xform( const trimesh::xform &xf );

	// A point cloud can be initialized from a file or adding vertices manually.
	// If a mesh file is used, the vertices of the mesh become the points of the cloud.
	// If "fill" is true, points will be added to fill the inner space of the mesh (TODO).
	// Currently supported file types are
	//	.ply	(triangle mesh)
	//	.node	(tet mesh w/o elements)
	// Returns true on success.
	bool load( std::string file, bool fill );

	// After vertices are added, you can compute radii instead of assigning them.
	// Radii will be assigned to each point based off a density estimator (delta).
	// delta should be between 0 and 1, with 0 being sparse and 1 being dense.
	void compute_radii( float delta );

private:
	std::string material;
	std::shared_ptr<AABB> aabb;

	// Fills a surface mesh with points
	void fill_mesh();
};


} // end namespace mcl

#endif
