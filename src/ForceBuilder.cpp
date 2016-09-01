// Copyright (c) 2016 University of Minnesota
// 
// ADMM-Elastic Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
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

#include "ForceBuilder.hpp"

using namespace admm;


int getUniqueVert(int fi, int f, std::shared_ptr<trimesh::TriMesh> mesh) {
	
	std::vector<bool> matches = {false,false,false};
	int matchCount = 0;
	
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			if( mesh.get()->faces[fi].v[i] == mesh.get()->faces[f].v[j] ){
				matches[i] = true;
				matchCount += 1;
				break;
			}
		}
	}
	
	if( matchCount != 2){
		std::cout << "Error in getUniqueVert: two input faces do not share 2 verts!\n";
		exit(0);
	} else {
		for(int i = 0; i < 3; i++){
			if( matches[i] == false ){
				return mesh.get()->faces[fi].v[i];
			}
		}
	}
	
	return -1;
	
}

bool isUniqueHinge( Eigen::Vector4i hv , const std::vector<Eigen::Vector4i>& hingeSignatures){
	
	bool unique = true;
	
	for(int i = 0; i < hingeSignatures.size(); i++){
		Eigen::Vector4i sigTemp = hingeSignatures[i];
		std::sort(sigTemp.data(),sigTemp.data()+sigTemp.size());
		std::sort(hv.data(),hv.data()+hv.size());

		//std::cout << "hv: " << hv[0] << " " << hv[1] << " " << hv[2] << " " << hv[3] << std::endl;
		//std::cout << "sig: " << sigTemp[0] << " " << sigTemp[1] << " " << sigTemp[2] << " " << sigTemp[3] << std::endl;

		if( hv == sigTemp ){
			unique = false;
		}	
	}
	
	return unique;
	
}

bool ForceBuilder::build_trimesh( std::shared_ptr<trimesh::TriMesh> mesh,
	mcl::Component &force, std::vector< std::shared_ptr<Force> > *sys_forces,
	int idx_offset ){

	using namespace trimesh;

	std::string force_type = mcl::parse::to_lower( force.type );
	mesh.get()->need_faces();


	std::vector<Eigen::Vector4i> hingeSignatures;

	// Map to store edges indices and avoid duplicate edges.
	// The int2 type stores both sorted and unsorted ints,
	// and the sorted (lowest first) is used in the map.
	// See mclscene's VertexSort.hpp for info.
	std::unordered_map< mcl::int2, int > edges_added;

	// Loop over the faces
	for( int f=0; f<mesh.get()->faces.size(); ++f ){

		TriMesh::Face face = mesh.get()->faces[f];
		int p0 = face.v[0]+idx_offset;
		int p1 = face.v[1]+idx_offset;
		int p2 = face.v[2]+idx_offset;
		vec faceNorm = ( mesh.get()->normals[face.v[0]] + mesh.get()->normals[face.v[1]] + mesh.get()->normals[face.v[2]] ) / 3.f;

		//
		//	Triangle Strain
		//
		if( force_type == "lineartrianglestrain" || force_type == "trianglestrain" ){

			// Parse parameters
			vec2 limit(0.f,9999999.f);
			if( force.exists("limit") ){ limit = force["limit"].as_vec2(); }
			if( !force.exists("stiffness") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a stiffness parameter." << std::endl;
				return false;
			}
			double stiffness = force["stiffness"].as_double();
			bool pd_style = false;
			if( force.exists("pd") ){ pd_style = force["pd"].as_bool(); } 

			if( pd_style ){
				// Projective dynamics triangle
				std::shared_ptr<Force> new_force( new
					PDTriangleStrain( p0, p1, p2, stiffness, limit[0], limit[1] )
				);
				sys_forces->push_back( new_force );
			} else {
				// Strain limited triangle
				std::shared_ptr<Force> new_force( new
					LimitedTriangleStrain( p0, p1, p2, stiffness, limit[0], limit[1] )
				);
				sys_forces->push_back( new_force );
			}

		} // end triangle strain


		//
		// Triangle bending forces
		//
		else if( force_type == "bend" ){
			
			if( !force.exists("stiffness") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a stiffness parameter." << std::endl;
				return false;
			}
			double stiffness = force["stiffness"].as_double();
			
			// Indices of verts in current face f
//			int p0 = face.v[0]+idx_offset;
//			int p1 = face.v[1]+idx_offset;
//			int p2 = face.v[2]+idx_offset;
			
			// Neighboring face indices (e.g., f0 is index of face opposite of p0)
			int f0 = mesh.get()->across_edge[f][0];
			int f1 = mesh.get()->across_edge[f][1];
			int f2 = mesh.get()->across_edge[f][2];
			
			// If the face across from vert p0 exists
			if( f0 >= 0 ){
	
				// f:f0 hinge verts in Volino ordering
				Eigen::Vector4i hv;
				hv[0] = p0;
				hv[1] = getUniqueVert(f0,f,mesh)+idx_offset;
				hv[2] = p2;
				hv[3] = p1;
				
				// If hinge is not already in system, make a bending force out of it
				if( isUniqueHinge(hv , hingeSignatures) ){
					std::shared_ptr<Force> new_force( new BendForce( hv[0], hv[1], hv[2], hv[3], stiffness ) );
					sys_forces->push_back( new_force );
					hingeSignatures.push_back( hv );
					bend_index += 1;
				}
				
			}
			
			// If the face across from vert p1 exists
			if( f1 >= 0 ){
				
				// f:f1 hinge verts in Volino ordering
				Eigen::Vector4i hv;
				hv[0] = p1;
				hv[1] = getUniqueVert(f1,f,mesh)+idx_offset;
				hv[2] = p0;
				hv[3] = p2;
				
				// If hinge is not already in system, make a bending force out of it
				if( isUniqueHinge(hv , hingeSignatures) ){
					std::shared_ptr<Force> new_force( new BendForce( hv[0], hv[1], hv[2], hv[3], stiffness ) );
					sys_forces->push_back( new_force );
					hingeSignatures.push_back( hv );
					bend_index += 1;
				}
			}
			
			// If the face across from vert p2 exists
			if( f2 >= 0 ){
				
				// f:f2 hinge verts in Volino ordering
				Eigen::Vector4i hv;
				hv[0] = p2;
				hv[1] = getUniqueVert(f2,f,mesh)+idx_offset;
				hv[2] = p1;
				hv[3] = p0;
				
				// If hinge is not already in system, make a bending force out of it
				if( isUniqueHinge(hv , hingeSignatures) ){
					std::shared_ptr<Force> new_force( new BendForce( hv[0], hv[1], hv[2], hv[3], stiffness ) );
					sys_forces->push_back( new_force );
					hingeSignatures.push_back( hv );
					bend_index += 1;
				}
			}
			
			
		} // end triangle bend



		//
		//	Springa-linga-ding-dong
		//
		else if( force_type == "spring" ){

			mcl::int2 edges[3] = { mcl::int2(p0,p1), mcl::int2(p0,p2), mcl::int2(p1,p2) };

			for( int e=0; e<3; ++e ){

				// Skip if the edge already exists
				if( edges_added.count( edges[e] )>0 ){ continue; }
				else{ edges_added.insert( std::make_pair( edges[e], 1 ) ); }

				// Parse parameters
				vec2 limit(-1.f,-1.f);
				if( force.exists("limit") ){ limit = force["limit"].as_vec2(); }
				if( !force.exists("stiffness") ){
					std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
					"\" needs a stiffness parameter." << std::endl;
					return false;
				}
				double stiffness = force["stiffness"].as_double();
				if( limit[0]>=0.f ){

					std::cout << "TODO: ForceBuilder::build_trimesh with limited springs" << std::endl;
					return false;
				}

				else{

					std::shared_ptr<Force> new_force( new
						Spring( edges[e][0], edges[e][1], stiffness )
					);

					sys_forces->push_back( new_force );

				}

			} // end loop edges

		} // end spring force

		else if( force_type != "constforce" ){
			std::cout << "TODO: ForceBuilder::build_trimesh with force: " << force_type << std::endl;
			return false;
		}

	} // end loop trimesh faces

	return true;

} // end build trimesh


bool ForceBuilder::build_tetmesh( std::shared_ptr<mcl::TetMesh> mesh,
	mcl::Component &force, std::vector< std::shared_ptr<Force> > *sys_forces,
	int idx_offset ){

	using namespace trimesh;

	std::string force_type = mcl::parse::to_lower( force.type );

	// Map to store edges indices and avoid duplicate edges.
	// The int2 type stores both sorted and unsorted ints,
	// and the sorted (lowest first) is used in the map.
	// See mclscene's VertexSort.hpp for info.
	std::unordered_map< mcl::int2, int > edges_added;

	// Loop over the tets
	for( int t=0; t<mesh->tets.size(); ++t ){

		// Indices to the points that make up the tet. 
		int p[4] = { mesh->tets[t].v[0]+idx_offset, mesh->tets[t].v[1]+idx_offset, mesh->tets[t].v[2]+idx_offset, mesh->tets[t].v[3]+idx_offset };

		if( force_type == "lineartetstrain" ){

			// Parse parameters
			vec2 limit(-1.f,-1.f);
			if( force.exists("limit") ){ limit = force["limit"].as_vec2(); }
			if( !force.exists("stiffness") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a stiffness parameter." << std::endl;
				return false;
			}
			double stiffness = force["stiffness"].as_double();
			double weight_scale = 1.0;
			if( force.exists("weight_scale") ){ weight_scale = force["weight_scale"].as_double(); }

				std::shared_ptr<Force> new_force( new
					LinearTetStrain( p[0], p[1], p[2], p[3], stiffness, weight_scale )
				);
				sys_forces->push_back( new_force );

		}
/*
		else if( force_type == "damping" ){
			
			assert( force.exists("damp_const") );
			assert( force.exists("mu") );
			assert( force.exists("lambda") );

			double damp_const = force["damp_const"].as_double();
			double mu = force["mu"].as_double();
			double lambda = force["lambda"].as_double();
			int max_iters = 10;
			if( force.exists("max_iterations") ){ max_iters = force["max_iterations"].as_int(); }	

			std::shared_ptr<Force> new_force( new
				DampingTet( p[0], p[1], p[2], p[3], damp_const, mu, lambda, max_iters )
			);
			sys_forces->push_back( new_force );
			
		}
*/
		else if( force_type == "neohookeantet" ){

			assert( force.exists("mu") );
			assert( force.exists("lambda") );

			double mu = force["mu"].as_double();
			double lambda = force["lambda"].as_double();
			int max_iters = 10;
			if( force.exists("max_iterations") ){ max_iters = force["max_iterations"].as_int(); }	

			std::shared_ptr<Force> new_force( new
				HyperElasticTet( p[0], p[1], p[2], p[3], mu, lambda, max_iters, "nh" )
			);
			sys_forces->push_back( new_force );

//std::cout << "tet: " << t << std::endl;

		}

		else if( force_type == "stvktet" ){

			assert( force.exists("mu") );
			assert( force.exists("lambda") );

			double mu = force["mu"].as_double();
			double lambda = force["lambda"].as_double();
			int max_iters = 10;
			if( force.exists("max_iterations") ){ max_iters = force["max_iterations"].as_int(); }	

			std::shared_ptr<Force> new_force( new
				HyperElasticTet( p[0], p[1], p[2], p[3], mu, lambda, max_iters, "stvk" )
			);
			sys_forces->push_back( new_force );

//std::cout << "tet: " << t << std::endl;

		}
		
		else if( force_type == "anisotropic" ){
			
			if( !force.exists("stiffness") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a stiffness parameter." << std::endl;
				return false;
			}
			
			if( !force.exists("k") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a k parameter." << std::endl;
				return false;
			}
			
			if( !force.exists("direction") ){
				std::cerr << "\n**ForceBuilder ERror: force \"" << force.name <<
				"\" needs a direction parameter." << std::endl;
				return false;
			}
			
			vec3 direction = force["direction"].as_vec3();
			Eigen::Vector3d e(direction[0],direction[1],direction[2]);
			double stiffness = force["stiffness"].as_double();
			double k = force["k"].as_double();
			
			std::shared_ptr<Force> new_force( new
				AnisotropicTet( p[0], p[1], p[2], p[3], e, k, stiffness )
			);
			sys_forces->push_back( new_force );

		}
		
		else if( force_type == "volpres" ){
			
			if( !force.exists("stiffness") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a stiffness parameter." << std::endl;
				return false;
			}
			
			if( !force.exists("range_min") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a range_min parameter." << std::endl;
				return false;
			}
			
			if( !force.exists("range_max") ){
				std::cerr << "\n**ForceBuilder Error: force \"" << force.name <<
				"\" needs a range_max parameter." << std::endl;
				return false;
			}
			
			double stiffness = force["stiffness"].as_double();
			double rangeMin = force["range_min"].as_double();
			double rangeMax = force["range_max"].as_double();
						
			std::shared_ptr<Force> new_force( new
				LinearTetVolume( p[0], p[1], p[2], p[3], rangeMin, rangeMax, stiffness )
			);
			sys_forces->push_back( new_force );
			
		}

		else if( force_type != "constforce" ){
			std::cout << "TODO: ForceBuilder::build_tetmesh with force: " << force_type << std::endl;
			return false;
		}

	} // end loop tets

	return true;

} // end add tet mesh

















std::shared_ptr<admm::System> ForceBuilder::system; // set by Context
std::unordered_map< std::string, mcl::Component > *ForceBuilder::force_param_map;
int ForceBuilder::index_offset;
int ForceBuilder::num_objects;
int ForceBuilder::bend_index=0;
std::unordered_map< int, std::pair< int, int > > *ForceBuilder::system_to_scene_map;










