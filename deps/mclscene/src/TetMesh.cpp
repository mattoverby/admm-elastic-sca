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

#include "MCL/TetMesh.hpp"
#include "MCL/VertexSort.hpp"
#include "tetgen.h"

using namespace mcl;

namespace tetmesh_helper {
	static std::string get_ext( std::string fname ){
		size_t pos = fname.find_last_of('.');
		return (std::string::npos == pos) ? "" : fname.substr(pos+1,fname.size());
	}
	static std::string to_lower( std::string s ){ std::transform( s.begin(), s.end(), s.begin(), ::tolower ); return s; }

	static char **make_argv( const std::vector<std::string> &s ){
		char **args;
		args = new char *[s.size()];
		for( int i = 0; i < s.size(); ++i ){
			args[i] = new char[s[i].length() + 1];
			strcpy( args[i], s[i].c_str());
		} 
		return args;
	}
} // end helper functions


bool TetMesh::load( std::string filename ){

	// Clear old data
	vertices.clear();
	tets.clear();
	normals.clear();
	faces.clear();

	// Get the extension
	std::string ext = tetmesh_helper::to_lower( tetmesh_helper::get_ext(filename) );

	// If it's a PLY we need to use tetgen
	if( ext=="ply" ){

		std::string new_filename = make_tetmesh( filename );
		if( new_filename.size()==0 ){ return false; }
		if( !load_node( new_filename ) ){ return false; }
		if( !load_ele( new_filename ) ){ return false; }
		if( !need_surface() ){ return false; }
	}

	else {
		// Load new data
		if( !load_node( filename ) ){ return false; }
		if( !load_ele( filename ) ){ return false; }
		if( !need_surface() ){ return false; }
	}

	need_normals();
	tris->need_tstrips();

	return true;
}


void TetMesh::need_normals( bool recompute ){

	if( vertices.size() == normals.size() && !recompute ){ return; }

	if( normals.size() != vertices.size() ){ normals.resize( vertices.size() ); }
	const int nv = normals.size();

#pragma omp parallel for
	for( int i = 0; i < nv; ++i ){
		normals[i][0] = 0.f; normals[i][1] = 0.f; normals[i][2] = 0.f;
	}

	int nf = faces.size();
#pragma omp parallel for
	for( int i = 0; i < nf; ++i ){
		const trimesh::point &p0 = vertices[faces[i][0]];
		const trimesh::point &p1 = vertices[faces[i][1]];
		const trimesh::point &p2 = vertices[faces[i][2]];
		trimesh::vec a = p0-p1, b = p1-p2, c = p2-p0;
		float l2a = trimesh::len2(a), l2b = trimesh::len2(b), l2c = trimesh::len2(c);
		if (!l2a || !l2b || !l2c)
			continue;
		trimesh::vec facenormal = a.cross( b );
		normals[faces[i][0]] += facenormal * (1.0f / (l2a * l2c));
		normals[faces[i][1]] += facenormal * (1.0f / (l2b * l2a));
		normals[faces[i][2]] += facenormal * (1.0f / (l2c * l2b));
	}

#pragma omp parallel for
	for (int i = 0; i < nv; i++){ trimesh::normalize(normals[i]); }

} // end compute normals


// Transform the mesh by the given matrix
void TetMesh::apply_xform( const trimesh::xform &xf ){
	int nv = vertices.size();
#pragma omp parallel for
	for (int i = 0; i < nv; i++){ vertices[i] = xf * vertices[i]; }

	if( !normals.empty() ){
		trimesh::xform nxf = trimesh::norm_xf(xf);
#pragma omp parallel for
		for (int i = 0; i < nv; i++) {
			normals[i] = nxf * normals[i];
			trimesh::normalize(normals[i]);
		}
	}
}


bool TetMesh::load_node( std::string filename ){

	// Load the vertices of the tetmesh
	std::stringstream node_file; node_file << filename << ".node";
	std::ifstream filestream;
	filestream.open( node_file.str().c_str() );
	if( !filestream ){ std::cerr << "\n**TetMesh Error: Could not load " << node_file.str() << std::endl; return false; }

	std::string header;
	getline( filestream, header );
	std::stringstream headerSS(header);
	int n_nodes = 0; headerSS >> n_nodes;

	vertices.resize( n_nodes );
	std::vector< int > vertex_set( n_nodes, 0 );
	bool starts_with_one = false;

	for( int i=0; i<n_nodes; ++i ){
		std::string line;
		getline( filestream, line );

		std::stringstream lineSS(line);
		double x, y, z;
		int idx;
		lineSS >> idx >> x >> y >> z;

		// Check for 1-indexed
		if( i==0 && idx==1 ){ starts_with_one = true; }
		if( starts_with_one ){ idx -= 1; }

		if( idx > vertices.size() ){
			std::cerr << "\n**TetMesh Error: Your indices are bad for file " << node_file.str() << std::endl; return false;
		}

		vertices[idx] = trimesh::point( x, y, z );
		vertex_set[idx] = 1;
	}
	filestream.close();

	for( int i=0; i<vertex_set.size(); ++i ){
		if( vertex_set[i] == 0 ){ std::cerr << "\n**TetMesh Error: Your indices are bad for file " << node_file.str() << std::endl; return false; }
	}

	return true;

} // end load node file

bool TetMesh::load_ele( std::string filename ){

	// Load the vertices of the tetmesh
	std::stringstream ele_file; ele_file << filename << ".ele";
	std::ifstream filestream;
	filestream.open( ele_file.str().c_str() );
	if( !filestream ){ std::cerr << "\n**TetMesh Error: Could not load " << ele_file.str() << std::endl; return false; }

	std::string header;
	getline( filestream, header );
	std::stringstream headerSS(header);
	int n_tets = 0; headerSS >> n_tets;

	tets.resize( n_tets );
	std::vector< int > tet_set( n_tets, 0 );
	bool starts_with_one = false;

	for( int i=0; i<n_tets; ++i ){
		std::string line;
		getline( filestream, line );

		std::stringstream lineSS(line);
		int idx;
		int node_ids[4];
		lineSS >> idx >> node_ids[0] >> node_ids[1] >> node_ids[2] >> node_ids[3];

		// Check for 1-indexed
		if( i==0 && idx==1 ){ starts_with_one = true; }
		if( starts_with_one ){
			idx -= 1;
			for( int j=0; j<4; ++j ){ node_ids[j]-=1; }
		}

		if( idx > tets.size() ){
			std::cerr << "\n**TetMesh Error: Your indices are bad for file " << ele_file.str() << std::endl; return false;
		}

		tets[idx] = tet( node_ids[0], node_ids[1], node_ids[2], node_ids[3] );
		tet_set[idx] = 1;
	}
	filestream.close();

	for( int i=0; i<tet_set.size(); ++i ){
		if( tet_set[i] == 0 ){ std::cerr << "\n**TetMesh Error: Your indices are bad for file " << ele_file.str() << std::endl; return false; }
	}

	return true;

} // end load ele file


bool TetMesh::need_surface(){

	// vertex ids -> number of faces using these indices
	std::unordered_map< int3, int > face_ids;

	// Loop over tets and store face information
	for( int t=0; t<tets.size(); ++t ){

		// Indices that make up the tetrahedra
		int p0 = tets[t].v[0];
		int p1 = tets[t].v[1];
		int p2 = tets[t].v[2];
		int p3 = tets[t].v[3];

		// Faces of each tetrahedra
		int3 curr_faces[4];
		curr_faces[0] = int3( p0, p1, p3 );
		curr_faces[1] = int3( p0, p2, p1 );
		curr_faces[2] = int3( p0, p3, p2 );
		curr_faces[3] = int3( p1, p2, p3 );

		// Store face count
		for( int f=0; f<4; ++f ){
			if( face_ids.count(curr_faces[f]) == 0 ){ face_ids[ curr_faces[f] ] = 1; }
			else{ face_ids[ curr_faces[f] ] += 1; }
		}
	}

	// Loop over face_ids and if a face only exists once (for all tets) its a boundary
	std::unordered_map< int3, int >::iterator faceIt = face_ids.begin();
	for( ; faceIt != face_ids.end(); ++faceIt ){
		if( faceIt->second == 1 ){
			int3 f = faceIt->first;
			faces.push_back( trimesh::TriMesh::Face( f.orig_v[0], f.orig_v[1], f.orig_v[2] ) );
		}
	}

	return true;

} // end create boundary mesh


void TetMesh::make_tri_refs(){

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


void TetMesh::bounds( trimesh::vec &bmin, trimesh::vec &bmax ){
	if( !aabb->valid ){
		for( int f=0; f<faces.size(); ++f ){
			(*aabb) += vertices[ faces[f][0] ];
			(*aabb) += vertices[ faces[f][1] ];
			(*aabb) += vertices[ faces[f][2] ];
		}
	}
	bmin = aabb->min; bmax = aabb->max;
}


void TetMesh::save( std::string filename ){

	//
	//	ele file
	//
	// http://wias-berlin.de/software/tetgen/fformats.ele.html
	// First line: <# of tetrahedra> <nodes per tetrahedron> <# of attributes>
	// Remaining lines list of # of tetrahedra:
	// <tetrahedron #> <node> <node> <node> <node> ... [attributes]
	{

		std::stringstream elefn;
		elefn << filename << ".ele";
		std::ofstream filestream;
		filestream.open( elefn.str().c_str() );
		filestream << tets.size() << " 4 0\n";
		for( int i=0; i<tets.size(); ++i ){
			filestream << "\t" << i << ' ' << tets[i].v[0] << ' ' << tets[i].v[1] << ' ' << tets[i].v[2] << ' ' << tets[i].v[3] << "\n";
		}
		filestream << "# Generated by mclscene (www.mattoverby.net)";
		filestream.close();

	} // end ele file

	//
	//	node file
	//
	// http://wias-berlin.de/software/tetgen/fformats.node.html
	// First line: <# of points> <dimension (must be 3)> <# of attributes> <# of boundary markers (0 or 1)>
	// Remaining lines list # of points:
	// <point #> <x> <y> <z> [attributes] [boundary marker]
	{

		std::stringstream nodefn;
		nodefn << filename << ".node";
		std::ofstream filestream;
		filestream.open( nodefn.str().c_str() );
		filestream << vertices.size() << " 3 0 0\n";
		for( int i=0; i<vertices.size(); ++i ){
			filestream << "\t" << i << ' ' << vertices[i].str() << "\n";
		}
		filestream << "# Generated by mclscene (www.mattoverby.net)";
		filestream.close();		

	} // end node file

} // end save


std::string TetMesh::get_xml( std::string name, int mode ){

	// Save to a NODE and ELE file
	std::stringstream nodeele;
	nodeele << MCLSCENE_BUILD_DIR<< "/" << name;
	save( nodeele.str() );
	
	// mclscene
	if( mode == 0 ){
		std::stringstream xml;
		xml << "\t<Object name=\"" << name << "\" type=\"TetMesh\" >\n";
		xml << "\t\t<File type=\"string\" value=\"" << nodeele.str() << "\" />\n";
		xml << "\t\t<Material type=\"string\" value=\"" << material << "\" />\n";
		xml << "\t</Object>";
		return xml.str();
	}

	return "";
}


std::string TetMesh::make_tetmesh( std::string filename ){

	std::cout << "\n******************************\n* Tetrahedralizing surface mesh. \n* " <<
		"Warning: This is buggy and you're better\n* off doing it yourself!" <<
		"\n******************************\n" << std::endl;

	std::string args = "./tetgen -q " + filename;
	std::vector<std::string> args_s;
	args_s.push_back( "./tetgen" );
	args_s.push_back( filename );
	args_s.push_back( "-F" ); // suppress .faces
	args_s.push_back( "-q" ); // quality mesh
	args_s.push_back( "-Q" ); // quiet terminal output
	char** args_c = tetmesh_helper::make_argv( args_s );

	tetgenio in, addin, bgmin;
	tetgenbehavior b;

	if( !b.parse_commandline( args_s.size(),args_c ) ){
		std::cerr << "\n**TetMesh::tetrahedralize Error: Trouble starting tetgen." << std::endl;
		return "";
	}
	if (b.refine) {
		if (!in.load_tetmesh(b.infilename)) {
			std::cerr << "\n**TetMesh::tetrahedralize Error: Error loading " << filename << std::endl;
			return "";
		}
	} else {
		if (!in.load_plc(b.infilename, (int) b.object)) {
			std::cerr << "\n**TetMesh::tetrahedralize Error: Error loading " << filename << std::endl;
			return "";
		}
	}
	if (b.insertaddpoints) {
		if (!addin.load_node(b.addinfilename)) {
			addin.numberofpoints = 0l;
		}
	}
	if (b.metric) {
		if (!bgmin.load_tetmesh(b.bgmeshfilename)) {
			bgmin.numberoftetrahedra = 0l;
		}
	}

	if (bgmin.numberoftetrahedra > 0l) {
		tetrahedralize(&b, &in, NULL, &addin, &bgmin);
	} else {
		tetrahedralize(&b, &in, NULL, &addin, NULL);
	}

	// Remove last 4 characters (ply)
	for( int i=0; i<4; ++i ){ filename.pop_back(); }
	std::string new_filename = filename + ".1";

	std::cout << "Saving mesh files: " << new_filename << " (.node and .ele)" << std::endl;
	std::cout << "\n******************************\n* Done running tetgen." <<
		"\n******************************\n" << std::endl;
	return new_filename;

}







