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

#include "SimContext.hpp"
#include "MCL/Application.hpp"
#include "ArgParser.hpp"

using namespace mcl;
using namespace admm;

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods );
void step_callback( System *system );

std::unique_ptr<SimContext> context;
std::unique_ptr<mcl::Application> app;
std::shared_ptr<ExplicitForce> wind;
Eigen::Vector3d orig_wind(10,0,2);

void setup();

bool high_winds = false;
bool both_at_once = false;
bool demo_mode = false; // demo mode runs the simulation and exits after 25 seconds

int main(int argc, char *argv[]){

	mcl::ArgParser args( argc, argv );
	if( args.exists("demo") ){ demo_mode = true; }
	if( args.exists("both") ){ both_at_once = true; }

	std::stringstream conf_ss;
	if( both_at_once ) { conf_ss << SRC_ROOT_DIR << "/samples/windyflag/together.xml"; }
	else{ conf_ss << SRC_ROOT_DIR << "/samples/windyflag/cloth.xml"; }

	// Create and initialize the dynamics context
	context = std::unique_ptr<SimContext>( new SimContext );
	context->load( conf_ss.str() );
	setup();
	context->settings.run_realtime = false; // export pngs and stitch together instead
	context->initialize();

	app = std::unique_ptr<mcl::Application>( new mcl::Application( context->scene.get(), context.get() ) );
	app->settings.gamma_correction = false;
	if( both_at_once ){
		app->zoom = 8.f;
		app->trans = trimesh::XForm<float>::trans( -1.f, 0.f, 0.f );
	} else { app->zoom = 6.f; }

	std::function<void (System*)> step_cb(step_callback);
	context->system->add_step_callback( step_cb );

	using namespace std::placeholders;    // adds visibility of _1, _2, _3,...
	Input::key_callbacks.push_back( std::bind( &key_callback, _1, _2, _3, _4, _5 ) );

	std::cout << "\n\n============\n\n\tPress W to toggle high winds\n\n============\n\n" << std::endl;

	app->display();

	return 0;
}



void setup(){

	using namespace trimesh;
	using namespace Eigen;

	// Get our meshes.
	TriMesh *cloth_m = context->scene->objects_map[ "cloth1" ]->get_TriMesh().get();

	// Pin the cloth corners
	std::vector< mcl::Param > cloth_params = context->scene->object_params["cloth1"];
	int cloth_width, cloth_height;
	for( int i=0; i<cloth_params.size(); ++i ){
		if( cloth_params[i].tag == "width" ){ cloth_width = cloth_params[i].as_int(); }
		if( cloth_params[i].tag == "length" ){ cloth_height = cloth_params[i].as_int(); }
	}
	int idx0 = 0;//(cloth_width+1)*(cloth_height+1)-1;
	int idx1 = cloth_height;

	std::cout << "Flag has " << cloth_m->faces.size() << " triangles" << std::endl;

	//
	//	Anchor Cloth
	//

	// Anchor the new nodes
	std::shared_ptr<Force> anchor_0( new StaticAnchor( idx0 ) );
	std::shared_ptr<Force> anchor_1( new StaticAnchor( idx1 ) );
	context->system->forces.push_back( anchor_0 );
	context->system->forces.push_back( anchor_1 );
	int idx_offset = cloth_m->vertices.size();

	if( both_at_once ){

		// Get our meshes.
		TriMesh *cloth_m2 = context->scene->objects_map[ "cloth2" ]->get_TriMesh().get();

		// Pin the cloth corners
		int cloth_width2, cloth_height2;
		std::vector< mcl::Param > cloth_params2 = context->scene->object_params["cloth2"];
		for( int i=0; i<cloth_params2.size(); ++i ){
			if( cloth_params2[i].tag == "width" ){ cloth_width2 = cloth_params2[i].as_int(); }
			if( cloth_params2[i].tag == "length" ){ cloth_height2 = cloth_params2[i].as_int(); }
		}

		int idx02 = idx_offset;//(cloth_width+1)*(cloth_height+1)-1;
		int idx12 = cloth_height2+idx_offset;

		//
		//	Anchor Cloth
		//

		// Anchor the new nodes
		std::shared_ptr<Force> anchor_02( new StaticAnchor( idx02 ) );
		std::shared_ptr<Force> anchor_12( new StaticAnchor( idx12 ) );
		context->system->forces.push_back( anchor_02 );
		context->system->forces.push_back( anchor_12 );

	}

	//
	//	Add wind manually so we can adjust the intensity with a button press
	//	Most of this is just copy-paste from SimContext
	//
	std::vector<int> faces;
	int total_sys_verts = 0;

	// Loop over all dynamic meshes in the scene, and create a vector of all faces.
	// This vector is used to create the wind force.
	std::unordered_map< std::string, std::vector<mcl::Param> >::iterator o_it = context->scene->object_params.begin();
	for( ; o_it != context->scene->object_params.end(); ++o_it ){

		bool has_force = false;
		for( int p=0; p<o_it->second.size(); ++p ){ if( o_it->second[p].tag=="force" ){ has_force=true; } }
		if( !has_force ){ continue; } // skip static objects

		std::shared_ptr<trimesh::TriMesh> mesh = context->scene->objects_map[ o_it->first ]->get_TriMesh();
		for( int f=0; f<mesh->faces.size(); ++f ){
			faces.push_back( mesh->faces[f][0]+total_sys_verts );
			faces.push_back( mesh->faces[f][1]+total_sys_verts );
			faces.push_back( mesh->faces[f][2]+total_sys_verts );
		}
		total_sys_verts += mesh->vertices.size();

	} // end loop dyanmic meshes

	// Now add the wind force
	wind = std::shared_ptr<admm::ExplicitForce>( new admm::WindForce(faces) );
	wind->direction = orig_wind;
	context->system->explicit_forces.push_back( wind );
}


void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ){

	if (action != GLFW_PRESS){ return; }
	if( key==GLFW_KEY_W ){ high_winds = !high_winds; }
}


// Adjust the wind setting to make it wavy and neato
float elapsed_s = 0.0;
bool ss_runonce = false;
void step_callback( System *system ){

	double xSpeed = 2.5;

	//
	//	Use W key to toggle high winds
	//
	if( !demo_mode ){

		// Update wind magnitude
		if( high_winds ){ wind->direction = orig_wind * xSpeed; }
		else{ wind->direction = orig_wind; }
		
	}


	//
	//	Time the high winds
	//
	else {

		// Run for a bit before starting the video
		if( elapsed_s > 4.9f && !ss_runonce ){ ss_runonce=true; app->settings.save_frames=true; }

		if( elapsed_s > 5.f+7.5f && elapsed_s < 5.f+15.f ){ wind->direction = orig_wind * xSpeed; }
		else{ wind->direction = orig_wind; }

		// Close after 25 seconds
		if( elapsed_s > 25.f ){ exit(1); }

	}

	elapsed_s += system->timestep_s;
}


