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

#ifndef ADMM_FORCEBUILDER_H
#define ADMM_FORCEBUILDER_H 1

#include "TriangleForce.hpp"
#include "BendForce.hpp"
#include "AnchorForce.hpp"
#include "TetForce.hpp"
#include "System.hpp"
#include "MCL/DefaultBuilders.hpp"
#include "MCL/VertexSort.hpp"

namespace admm {

//
//	ForceBuilder is a factory class for generating system forces
//	for a given mesh or object. It's a bit clunky, but wasn't
//	meant for long term use (just to throw together our samples).
//
class ForceBuilder {
public:

	// Because the ForceBuilder uses static variables, the context
	// needs to reset these variables from its destructor to allow
	// multiple context creation/deletions.
	static void reset(){
		index_offset=0;
		num_objects=0;
		bend_index=0;
	}

	static bool build_trimesh(
		std::shared_ptr<trimesh::TriMesh> mesh,
		mcl::Component &force,
		std::vector< std::shared_ptr<Force> > *sys_forces,
		int idx_offset );

	static bool build_tetmesh(
		std::shared_ptr<mcl::TetMesh> mesh,
		mcl::Component &force,
		std::vector< std::shared_ptr<Force> > *sys_forces,
		int idx_offset );

	static std::shared_ptr<admm::System> system;
	static int index_offset;
	static int num_objects;
	static int bend_index;
	static std::unordered_map< std::string, mcl::Component > *force_param_map;
	static std::unordered_map< int, std::pair< int, int > > *system_to_scene_map;
	static std::unordered_map< std::string, std::vector< std::string > > obj_to_forces;


	// This callback function is bound to the SceneManager and called when and Object component
	// is parsed. In the SceneManager library there is a default builder
	// function that creates everything as a triangle/tetrahedral mesh that
	// we can use. Then, we can add nodes/forces to the system.
	static std::shared_ptr<mcl::BaseObject> admm_build_object( mcl::Component &obj ){

		num_objects++;
		using namespace mcl;
		std::string o_type = parse::to_lower(obj.type);
		std::string o_name = obj.name;

		// This function will convert everything (sphers, boxes, etc...) to
		// either a triangle mesh or a tetmesh.
		std::shared_ptr<mcl::BaseObject> object = default_build_object( obj );
		std::shared_ptr<trimesh::TriMesh> mesh = object->get_TriMesh(); // everything can be casted to a TriMesh though...
		mesh->need_normals();

		//
		//	If the object doesn't have a "Force" component, it's static and we're done!
		//
		if( !obj.exists("force") ){ return object; }


		//
		//	Get important information from the Object component
		//	(e.g. mass, init parameters, etc...)
		//
		double objMass = -1.0;
		if( obj.exists("mass") ){ objMass = obj.get("mass").as_double(); }
		if( objMass < 0.0 ){
			std::cerr << "\n**Error: You must specify mass (kg) for object "
				<< o_name << ", e.g. <Mass type=\"double\" value=\"2\" />" << std::endl;
			assert(false);
		} double node_mass = objMass / mesh->vertices.size();


		//
		//	In most cases we want to use density weighted masses.
		//	However, to compare to other methods (projective dynamics)
		//	we need to use uniform mass values.
		//
		bool density_weighted_mass=true;
		if( obj.exists("density_weighted_mass") ){ density_weighted_mass = obj.get("density_weighted_mass").as_bool(); }


		//
		//	Add Nodes to the system
		//
		int old_system_nodes = system->m_x.size()/3;
		system->m_x.conservativeResize((old_system_nodes+mesh->vertices.size())*3);
		system->m_v.conservativeResize((old_system_nodes+mesh->vertices.size())*3);
		system->m_masses.conservativeResize((old_system_nodes+mesh->vertices.size())*3);

		for( int i=0; i<mesh->vertices.size(); ++i ){
			int sys_idx = old_system_nodes + i;

			// Store the map info.
			std::pair<int,int> scene_mesh_idx = std::make_pair( (num_objects-1), i );
			system_to_scene_map->insert( std::make_pair( sys_idx, scene_mesh_idx ) );

			// Copy over node location
			trimesh::point p = mesh->vertices[i];
			system->m_x[ sys_idx*3 + 0 ] = p[0];
			system->m_x[ sys_idx*3 + 1 ] = p[1];
			system->m_x[ sys_idx*3 + 2 ] = p[2];

			// Set node mass
			if( density_weighted_mass ){
				system->m_masses[ sys_idx*3 + 0 ] = 0.f;
				system->m_masses[ sys_idx*3 + 1 ] = 0.f;
				system->m_masses[ sys_idx*3 + 2 ] = 0.f;
			} else {
				system->m_masses[ sys_idx*3 + 0 ] = node_mass;
				system->m_masses[ sys_idx*3 + 1 ] = node_mass;
				system->m_masses[ sys_idx*3 + 2 ] = node_mass;
			}

			// Set node normals
			trimesh::vec n = mesh->normals[i];

		} // end copy node information


		//
		//	Add Forces to the system
		//
		for( int i=0; i<obj.params.size(); ++i ){

			// Get a force we want to use for this object
			std::string f_tag = parse::to_lower( obj.params[i].tag );
			if( f_tag != "force" ){ continue; }

			// Make sure the force name exists in the system
			std::string f_name = obj.params[i].value;
			if( force_param_map->count( f_name )==0 ){
				std::cerr << "\n**ForceBuilder::Error: No force named \"" << f_name << "\" for object \"" << o_name << "\"" << std::endl;
				assert(false);
			}

			mcl::Component force = force_param_map->at( f_name );

			// It's either a triangle mesh or a tet mesh
			if( o_type == "tetmesh" ){
				std::shared_ptr<mcl::TetMesh> t_mesh = std::static_pointer_cast<mcl::TetMesh>(object);
				std::cout << "Tetmesh " << o_name << " has " << t_mesh->tets.size() << " tets." << std::endl;
				build_tetmesh( t_mesh, force, &system->forces, index_offset );
			} // end create tet mesh forces

			else { // create triangle mesh forces
				using namespace trimesh;
				std::cout << "Trimesh " << o_name << " has " << mesh->faces.size() << " tris." << std::endl;
				build_trimesh( mesh, force, &system->forces, index_offset ); 
			} // end create triangle mesh forces

		} // end loop params


		//
		//	Now compute masses based on area/volume weighting
		//
		if( density_weighted_mass ){

			//
			//	Tet Mesh
			//
			if( o_type == "tetmesh" ){
				std::shared_ptr<mcl::TetMesh> t_mesh = std::static_pointer_cast<mcl::TetMesh>(object);

				double totVolume = 0;
				for(int t=0; t<t_mesh->tets.size(); t++){
					int p[4] = { t_mesh->tets[t].v[0]+index_offset, t_mesh->tets[t].v[1]+index_offset, 
							 t_mesh->tets[t].v[2]+index_offset, t_mesh->tets[t].v[3]+index_offset };
					Eigen::Vector3d v0( system->m_x[ p[0]*3 ], system->m_x[ p[0]*3+1 ], system->m_x[ p[0]*3+2 ] );
					Eigen::Vector3d v1( system->m_x[ p[1]*3 ], system->m_x[ p[1]*3+1 ], system->m_x[ p[1]*3+2 ] );
					Eigen::Vector3d v2( system->m_x[ p[2]*3 ], system->m_x[ p[2]*3+1 ], system->m_x[ p[2]*3+2 ] );
					Eigen::Vector3d v3( system->m_x[ p[3]*3 ], system->m_x[ p[3]*3+1 ], system->m_x[ p[3]*3+2 ] );
					totVolume += fabs( (v0-v3).dot( (v1-v3).cross(v2-v3) ) ) / 6.0;
				}
			
				double density = 0;
				if( totVolume > 0 ){
					density = objMass / totVolume;
				} else {
					std::cerr << "\n**Error: tet object volume is zero, so can't compute mass density. \n";
					assert(false);
				}
			
				for(int t=0; t<t_mesh->tets.size(); t++){
					int p[4] = { t_mesh->tets[t].v[0]+index_offset, t_mesh->tets[t].v[1]+index_offset, 
							 t_mesh->tets[t].v[2]+index_offset, t_mesh->tets[t].v[3]+index_offset };
					Eigen::Vector3d v0( system->m_x[ p[0]*3 ], system->m_x[ p[0]*3+1 ], system->m_x[ p[0]*3+2 ] );
					Eigen::Vector3d v1( system->m_x[ p[1]*3 ], system->m_x[ p[1]*3+1 ], system->m_x[ p[1]*3+2 ] );
					Eigen::Vector3d v2( system->m_x[ p[2]*3 ], system->m_x[ p[2]*3+1 ], system->m_x[ p[2]*3+2 ] );
					Eigen::Vector3d v3( system->m_x[ p[3]*3 ], system->m_x[ p[3]*3+1 ], system->m_x[ p[3]*3+2 ] );
					double volume = fabs( (v0-v3).dot( (v1-v3).cross(v2-v3) ) ) / 6.0;
					double tetMass = density * volume;
				
					for(int j = 0; j < 4; j++){
						system->m_masses[ p[j]*3 ] += tetMass / 4.0;
						system->m_masses[ p[j]*3 + 1 ] += tetMass / 4.0;
						system->m_masses[ p[j]*3 + 2 ] += tetMass / 4.0;
					}
				
				}


			} // end tet mesh

			//
			//	Triangle Mesh
			//
			else { // otherwise its a triangle mesh
				using namespace trimesh;

				double totArea = 0;
				for(int f=0; f<mesh.get()->faces.size(); f++){
					TriMesh::Face face = mesh.get()->faces[f];
					int p[3] = { face.v[0]+index_offset, face.v[1]+index_offset, face.v[2]+index_offset };

					Eigen::Vector3d v0( system->m_x[ p[0]*3 ], system->m_x[ p[0]*3+1 ], system->m_x[ p[0]*3+2 ] );
					Eigen::Vector3d v1( system->m_x[ p[1]*3 ], system->m_x[ p[1]*3+1 ], system->m_x[ p[1]*3+2 ] );
					Eigen::Vector3d v2( system->m_x[ p[2]*3 ], system->m_x[ p[2]*3+1 ], system->m_x[ p[2]*3+2 ] );

					totArea += 0.5 * ((v1-v0).cross(v2-v0)).norm();
				
				}
			
				double density = 0;
				if( totArea > 0 ){
					std::cout << "totalArea is .. " << totArea << std::endl;
					std::cout << "objMass is .. " << objMass << std::endl;
					density = objMass / totArea;
				} else {
					std::cerr << "\n**Error: tri object area is zero, so can't compute mass density. \n";
					assert(false);
				}
			
				for(int f=0; f<mesh.get()->faces.size(); f++){
				
					TriMesh::Face face = mesh.get()->faces[f];
					int p[3] = { face.v[0]+index_offset, face.v[1]+index_offset, face.v[2]+index_offset };

					Eigen::Vector3d v0( system->m_x[ p[0]*3 ], system->m_x[ p[0]*3+1 ], system->m_x[ p[0]*3+2 ] );
					Eigen::Vector3d v1( system->m_x[ p[1]*3 ], system->m_x[ p[1]*3+1 ], system->m_x[ p[1]*3+2 ] );
					Eigen::Vector3d v2( system->m_x[ p[2]*3 ], system->m_x[ p[2]*3+1 ], system->m_x[ p[2]*3+2 ] );

					double triArea = 0.5 * ((v1-v0).cross(v2-v0)).norm();
					//std::cout << "triArea: " << triArea << std::endl;
					//std::cout << "density: " << density << std::endl;
				

					double triMass = density * triArea;
				
					for(int j = 0; j < 3; j++){
						system->m_masses[ p[j]*3 ] += triMass / 3.0;
						system->m_masses[ p[j]*3 + 1 ] += triMass / 3.0;
						system->m_masses[ p[j]*3 + 2 ] += triMass / 3.0;
					}
							
				}
			
				double runningSum = 0;
				for(int j = 0; j < system->m_masses.size(); j++){
					runningSum += system->m_masses[j];
					//std::cout << "system->m_masses[j] is .. " << system->m_masses[j] << std::endl;
				}
				runningSum = runningSum / 3.0;

				std::cout << "tot mass computed: " << runningSum << std::endl;

			} // end compute area weighted mass

		} // end density weighted mass


		index_offset += mesh->vertices.size();
		return object;

	} // end build object

};


}

#endif
