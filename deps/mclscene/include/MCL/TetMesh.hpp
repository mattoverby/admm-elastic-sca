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

#ifndef MCLSCENE_TETMESH_H
#define MCLSCENE_TETMESH_H 1

#include "MCL/TriangleMesh.hpp"

namespace mcl {

//
//	Tetrahedral Mesh
//
class TetMesh : public BaseObject {
private: std::shared_ptr<trimesh::TriMesh> tris; // tris is actually the data container
public:
	struct tet {
		tet(){}
		tet( int a, int b, int c, int d ){ v[0]=a; v[1]=b; v[2]=c; v[3]=d; }
		int v[4];
	};

	std::vector< tet > tets; // all elements
	std::vector< trimesh::point > &vertices; // all vertices in the tet mesh
	std::vector< trimesh::vec > &normals; // zero length for all non-surface normals
	std::vector< trimesh::TriMesh::Face > &faces; // surface triangles

	TetMesh( std::string mat="" ) : tris(new trimesh::TriMesh), vertices(tris->vertices), normals(tris->normals), faces(tris->faces),
		material(mat), aabb(new AABB) {}

	std::string get_type() const { return "tetmesh"; }

	// Filename is the first part of a tetmesh which must contain an .ele and .node file.
	// If a ply file is supplied, tetgen will be used to tetrahedralize the mesh (however,
	// the ply must be ascii, not binary).
	// Returns true on success
	bool load( std::string filename );

	// Saves the tet mesh to .ele and .node files.
	// Do not include extensions on the filename argument.
	void save( std::string filename );

	std::string get_xml( std::string name, int mode );

	// Compute the normals for surface vertices. The inner normals are length zero.
	void need_normals( bool recompute=true );

	// Transform the mesh by the given matrix
	void apply_xform( const trimesh::xform &xf );

	// Creates a new trimesh object from ALL vertices and stuff
	const std::shared_ptr<trimesh::TriMesh> get_TriMesh(){ return tris; }

	std::string get_material() const { return material; }
	void set_material( std::string mat ){ material=mat; }

	void bounds( trimesh::vec &bmin, trimesh::vec &bmax );

	void update(){ aabb->valid=false; }

	void get_primitives( std::vector< std::shared_ptr<BaseObject> > &prims ){
		if( tri_refs.size() != faces.size() ){ make_tri_refs(); }
		prims.insert( prims.end(), tri_refs.begin(), tri_refs.end() );
	}

private:
	std::string material;
	std::shared_ptr<AABB> aabb;

	bool load_node( std::string filename );

	bool load_ele( std::string filename );

	// Computes a surface mesh, called by load
	bool need_surface();

	// Uses tetgen to tetrahedralize a mesh, returning
	// the filename of the new files (node and ele)
	// which are generated and dumped in the same directory
	// as the original ply.
	// Returns an empty string on failure.
	std::string make_tetmesh( std::string filename );

	// Triangle refs are used for BVH hook-in.
	void make_tri_refs();
	std::vector< std::shared_ptr<BaseObject> > tri_refs;

}; // end class TetMesh

} // end namespace mcl

#endif
