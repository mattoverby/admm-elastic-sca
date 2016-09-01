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

#include "CollisionCylinder.hpp"
#include "CollisionForce.hpp"

using namespace mcl;
using namespace admm;

void setup();
std::unique_ptr<SimContext> context;

int main(int argc, char *argv[]){

	std::stringstream conf_ss; conf_ss << SRC_ROOT_DIR << "/samples/plinkopony/plinko.xml";

	// Create and initialize the dynamics context
	context = std::unique_ptr<SimContext>( new SimContext() );
	context->load( conf_ss.str() );
	setup();
	context->initialize();

	mcl::Application app( context->scene.get(), context.get() );
	app.settings.gamma_correction = false;
	app.trans = trimesh::XForm<float>::trans(0,-5.f,0);
	app.zoom = 20.f;
//	gui.set_init_xform( trimesh::xform::trans( -0.6f, 1.3f, 32.f ) );
	app.display();

	return 0;
}


void setup(){	
	
	using namespace trimesh;
	
	double stiffness = 1000;
	
	std::vector< std::shared_ptr<CollisionShape> > shapes;
	
	/* CYLINDERS */
	{

		std::unordered_map< std::string, std::vector<mcl::Param> >::iterator it = context->scene->object_params.begin();
		for( ; it != context->scene->object_params.end(); ++it ){

			// All of the cylinders are called "cyl<number>"
			if( it->first[0]!='c' ){ continue; }

			// Extract the parameters from the XML file to create the collision object
			double rad = 1.f;
			Eigen::Vector3d center(0,0,0), scale(1,1,1);
			for( int i=0; i<it->second.size(); ++i ){

				if( it->second[i].tag == "scale_copy" ){
					trimesh::vec v=it->second[i].as_vec3();
					scale = Eigen::Vector3d( v[0], v[1], v[2] );
				}
				else if( it->second[i].tag == "translate_copy" ){
					trimesh::vec v=it->second[i].as_vec3();
					center = Eigen::Vector3d( v[0], v[1], v[2] );
				}
				else if( it->second[i].tag == "radius" ){ rad = it->second[i].as_double(); }
			}

			std::shared_ptr<CollisionShape> shape( new CollisionCylinder(center,scale,rad) );
			shapes.push_back( shape );

		} // end loop objects
		
	}
	
	
	std::shared_ptr<Force> force( new CollisionForce( shapes ) );
	context->system->forces.push_back( force );	
}



