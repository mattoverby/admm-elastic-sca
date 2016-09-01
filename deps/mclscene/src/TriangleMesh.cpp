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

#include "MCL/TriangleMesh.hpp"

using namespace mcl;

TriangleMesh::TriangleMesh( std::shared_ptr<trimesh::TriMesh> tm, std::string mat ) :
	tris(tm), vertices(tm->vertices), normals(tm->normals), faces(tm->faces),
	material(mat), aabb(new AABB) {

	// Build triangle refs if they don't exist yet
	make_tri_refs();
	for( int f=0; f<faces.size(); ++f ){
		(*aabb) += vertices[ faces[f][0] ];
		(*aabb) += vertices[ faces[f][1] ];
		(*aabb) += vertices[ faces[f][2] ];
	}

} // end constructor


TriangleMesh::TriangleMesh( std::string mat ) : tris(new trimesh::TriMesh()),
	vertices(tris->vertices), normals(tris->normals), faces(tris->faces),
	material(mat), aabb(new AABB) {
} // end constructor


bool TriangleMesh::load( std::string filename ){

	// Clear old data
	vertices.clear();
	normals.clear();
	faces.clear();
	tri_refs.clear();

	// Load the new mesh and copy over data
	std::shared_ptr<trimesh::TriMesh> newmesh( trimesh::TriMesh::read( filename.c_str() ) );
	if( newmesh.get() == NULL ){ return false; }

	trimesh::remove_unused_vertices( newmesh.get() );
	vertices = newmesh->vertices;
	faces = newmesh->faces;
	tris->need_normals();
	tris->need_tstrips();

	// Remake the triangle refs and aabb
	make_tri_refs();
	for( int f=0; f<faces.size(); ++f ){
		(*aabb) += vertices[ faces[f][0] ];
		(*aabb) += vertices[ faces[f][1] ];
		(*aabb) += vertices[ faces[f][2] ];
	}

	return true;

} // end load file


void TriangleMesh::bounds( trimesh::vec &bmin, trimesh::vec &bmax ){
	if( !aabb->valid ){
		for( int f=0; f<faces.size(); ++f ){
			(*aabb) += vertices[ faces[f][0] ];
			(*aabb) += vertices[ faces[f][1] ];
			(*aabb) += vertices[ faces[f][2] ];
		}
	}
	bmin = aabb->min; bmax = aabb->max;
}


void TriangleMesh::apply_xform( const trimesh::xform &xf ){
	trimesh::apply_xform( tris.get(), xf );
	// Update the bounding box
	aabb->valid = false;
	for( int f=0; f<faces.size(); ++f ){
		(*aabb) += vertices[ faces[f][0] ];
		(*aabb) += vertices[ faces[f][1] ];
		(*aabb) += vertices[ faces[f][2] ];
	}
}


void TriangleMesh::make_tri_refs(){

	using namespace trimesh;

	tri_refs.clear();

	// Create the triangle reference objects
	tris->need_faces();
	tris->need_normals();

	for( int i=0; i<faces.size(); ++i ){
		TriMesh::Face f = faces[i];
		std::shared_ptr<BaseObject> tri(
			new TriangleRef( &vertices[f[0]], &vertices[f[1]], &vertices[f[2]], &normals[f[0]], &normals[f[1]], &normals[f[2]], material )
		);
		tri_refs.push_back( tri );
	} // end loop faces

} // end make triangle references

/*
bool TriangleMesh::ray_intersect( const intersect::Ray &ray, intersect::Payload &payload ) const {

	// Check aabb first
	if( !intersect::ray_aabb( ray, aabb->min, aabb->max, payload ) ){ return false; }

	bool hit = false;
	for( int i=0; i<tri_refs.size(); ++i ){
		if( tri_refs[i]->ray_intersect( ray, payload ) ){ hit = true; }
	} // end loop tris

	return hit;
}
*/

std::string TriangleMesh::get_xml( std::string obj_name, int mode ){

	// Save the PLY
	std::stringstream plyfile;
	plyfile << MCLSCENE_BUILD_DIR<< "/" << obj_name << ".ply";
	tris->write( plyfile.str() );

	// mclscene
	if( mode == 0 ){
		std::stringstream xml;
		xml << "\t<Object name=\"" << obj_name << "\" type=\"TriMesh\" >\n";
		xml << "\t\t<File value=\"" << plyfile.str() << "\" />\n";
		xml << "\t\t<Material value=\"" << material << "\" />\n";
		xml << "\t</Object>";
		return xml.str();
	}

	return "";
}

