// Copyright (c) 2017 University of Minnesota
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

#include "SimContext.hpp"

SimContext::SimContext(){

	scene = std::shared_ptr<mcl::SceneManager>( new mcl::SceneManager() );
	system = std::shared_ptr<admm::System>( new admm::System() );

	// Bind new builder functions to the SceneManager
	// When the parser loads an object, it will also create forces for
	// elements of the object (e.g., Tet forces, triangle forces, etc...)
	// and add them to the admm::System.
	admm::ForceBuilder::system = system;
	admm::ForceBuilder::system_to_scene_map = &system_to_scene_map;
	admm::ForceBuilder::force_param_map = &force_param_map;
	scene->createObject = admm::ForceBuilder::admm_build_object;

} // end constructor


void SimContext::load( std::string config_file ){

	//
	//	First, we want to load any force properties and store them in force_param_map
	//
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(config_file.c_str());
	if( !result ){ throw std::runtime_error("\n**SimContext::load Error: Unable to load "+config_file); }

	// Get the node that stores sim info
	pugi::xml_node head_node = doc.first_child();
	while( mcl::parse::to_lower(head_node.name()) != "admmelastic" && head_node ){ head_node = head_node.next_sibling(); }

	// Now parse simulation information
	for( pugi::xml_node::iterator main_node = head_node.begin(); main_node != head_node.end(); ++main_node ){
		pugi::xml_node curr_node = *main_node;
		std::string name = curr_node.attribute("name").as_string();
		std::string type = curr_node.attribute("type").as_string();
		std::string tag = mcl::parse::to_lower( curr_node.name() );

		if( tag=="solver" ){

			// Load the parameters
			std::vector<mcl::Param> params;
			mcl::load_params( params, curr_node );

			// Loop over them and set relavent ones
			for( int i=0; i<params.size(); ++i ){

				if( params[i].tag=="iterations" ){ system->settings.admm_iters = params[i].as_int(); } 
				else if( params[i].tag=="timestep" ){ system->settings.timestep_s = params[i].as_double(); }
				else if( params[i].tag=="realtime" ){ settings.run_realtime = params[i].as_bool(); }
				else if( params[i].tag=="verbose" ){ system->settings.verbose = params[i].as_int(); }

			} // end loop params

		} // end is solver info
		else if( tag=="force" ){

			if( name.size() == 0 || type.size() == 0 ){
				throw std::runtime_error("\n**SimContext::load Error: Force \""+tag+"\" need a name and type.");
			}

			// Load the parameters
			std::vector<mcl::Param> params;
			mcl::load_params( params, curr_node );

			// Create the component
			mcl::Component c( tag, name, type );
			c.params = std::move(params);
			force_param_map.insert( std::make_pair( name, c ) );

		} // end add force

	} // end loop scene info

	//
	//	Now we can load the scene and invoke the creator callbacks
	//	(it has its own error messages)
	//
	if( !scene->load( config_file ) ){ throw std::runtime_error("\nExiting..."); }


} // end load config file


void SimContext::initialize(){

	//
	//	Loop over the force_param_map and gravity, wind, or anchor forces forces
	//	This happens at initialize, because wind forces are applied to all triangles
	//	(which we need to know about to add the wind forces).
	//
	std::unordered_map< std::string, mcl::Component >::iterator f_it = force_param_map.begin();
	for( ; f_it != force_param_map.end(); ++f_it ){
		std::string type = mcl::parse::to_lower(f_it->second.type);

		if( type=="explicitforce" ){
			std::shared_ptr<admm::ExplicitForce> ef( new admm::ExplicitForce() );
			trimesh::vec v = f_it->second.get("direction").as_vec3();
			for( int j=0; j<3; ++j ){ ef->direction[j] = v[j]; }
			system->explicit_forces.push_back( ef );
		}


		else if( type=="staticanchor" ){
			int idx = f_it->second.get("index").as_int();
			std::shared_ptr<admm::Force> af( new admm::StaticAnchor( idx ) );
			system->forces.push_back( af );
		}


		else if( type=="windforce" || type=="wind" ){

			std::vector<int> faces;
			int total_sys_verts = 0;

			// Loop over all dynamic meshes in the scene, and create a vector of all faces.
			// This vector is used to create the wind force.
			std::unordered_map< std::string, std::vector<mcl::Param> >::iterator o_it = scene->object_params.begin();
			for( ; o_it != scene->object_params.end(); ++o_it ){

				bool has_force = false;
				for( int p=0; p<o_it->second.size(); ++p ){ if( o_it->second[p].tag=="force" ){ has_force=true; } }
				if( !has_force ){ continue; } // skip static objects

				std::shared_ptr<trimesh::TriMesh> mesh = scene->objects_map[ o_it->first ]->get_TriMesh();
				if( mesh==NULL ){ throw std::runtime_error("\nSimContext::initialize Error: Problem with mesh creation."); }

				for( int f=0; f<mesh->faces.size(); ++f ){
					faces.push_back( mesh->faces[f][0]+total_sys_verts );
					faces.push_back( mesh->faces[f][1]+total_sys_verts );
					faces.push_back( mesh->faces[f][2]+total_sys_verts );
				}
				total_sys_verts += mesh->vertices.size();

			} // end loop dyanmic meshes

			// Now add the wind force
			std::shared_ptr<admm::ExplicitForce> cf( new admm::WindForce(faces) );
			trimesh::vec v = f_it->second.get("direction").as_vec3();
			for( int j=0; j<3; ++j ){ cf->direction[j] = v[j]; }
			system->explicit_forces.push_back( cf );
		}


	} // end add other force types


	//
	// Finalize the system. It has own error messages
	//
	if( !system->initialize() ){ throw std::runtime_error("\nExiting..."); }

} // end init


bool SimContext::update( mcl::SceneManager *scene_ ){

	using map_it=std::unordered_map< int, std::pair< int, int > >::const_iterator;

#pragma omp parallel for
	for( int i=0; i<system_to_scene_map.size(); ++i ){
		map_it it = system_to_scene_map.begin(); std::advance(it, i); // even though advance is slow, it's needed for parallel map loopage
		trimesh::point p( system->m_x[ it->first*3 + 0 ], system->m_x[ it->first*3 + 1 ], system->m_x[ it->first*3 + 2 ] );
		std::shared_ptr<trimesh::TriMesh> mesh = scene->objects[ it->second.first ]->get_TriMesh();
		if( mesh==NULL ){ throw std::runtime_error("\nSimContext::update Error, something went wrong..."); }
		mesh->vertices[ it->second.second ] = p;
	} // end loop map

#pragma omp parallel for
	for( int i=0; i<scene->objects.size(); ++i ){
		scene->objects[i]->update();
	}

	return true;
}


bool SimContext::step( const mcl::SceneManager *scene_, float screen_dt ){

	if( !settings.run_realtime ){ return system->step(); }

	double timeleft = screen_dt;
	while( timeleft > 0.0 ){
		if( !system->step() ){ return false; }
		timeleft -= system->settings.timestep_s;
	}

	return true;

}


