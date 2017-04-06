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

using namespace mcl;
using namespace admm;

class grabber_sphere {
public:
	grabber_sphere() : render_obj(NULL) {}

	std::shared_ptr<mcl::BaseObject> render_obj;
	trimesh::point c;
	double rad;

	std::vector< Eigen::VectorXd > start_points, end_points;
	std::vector< std::shared_ptr<ControlPoint> > control_points;

	bool in_sphere( trimesh::point pt ){
		if( trimesh::len(pt-c) < rad ){
			start_points.push_back( Eigen::Vector3d(pt[0],pt[1],pt[2]) );
			std::shared_ptr<ControlPoint> p( new ControlPoint(Eigen::Vector3d(pt[0],pt[1],pt[2])) );
			control_points.push_back( p );
			return true;
		}
		return false;
	}
	void make_end_points( trimesh::point end_pos ){
		trimesh::vec disp = end_pos - c;
		Eigen::Vector3d displace( disp[0], disp[1], disp[2] );
		for( int i=0; i<start_points.size(); ++i ){ end_points.push_back( start_points[i]+displace ); }
	}

	void update( double elapsed_s, double start_s, double end_s ){
		assert( start_points.size() == end_points.size() );
		for( int i=0; i<start_points.size(); ++i ){
			Eigen::Vector3d pos = admm::helper::smooth_move( elapsed_s, start_s, end_s, start_points[i], end_points[i] );
			control_points[i]->pos = pos;
		}
	}

};



void setup();
void draw_callback( GLFWwindow* window, RenderGL::AppCamera *cam, float screen_dt );
void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods );
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);
//void event_callback( sf::Event &event );

std::unique_ptr<SimContext> context;
std::unique_ptr<mcl::Application> app;
std::shared_ptr<BaseMaterial> ball_mat;
grabber_sphere hand, foot;
float scene_rad;
bool activeFoot = true;
bool activeHand = true;
bool movingHand = true;
bool movingFoot = false;
trimesh::Vec<2,int> oldMousePos(-1,-1);

int main(int argc, char *argv[]){

	std::stringstream conf_ss; conf_ss << SRC_ROOT_DIR << "/samples/poordillo/poordillo.xml";

	// Create and initialize the dynamics context
	context = std::unique_ptr<SimContext>( new SimContext() );
	context->load( conf_ss.str() );
	setup();
	context->system->settings.verbose=0;
	context->settings.run_realtime = true;
	context->initialize();

	for(int i = 0; i < hand.control_points.size(); i++){
		hand.control_points[i] -> active = activeHand;
	}
	for(int i = 0; i < foot.control_points.size(); i++){
		foot.control_points[i] -> active = activeFoot;
	}

	scene_rad = context->scene->radius();

	std::cout << "\n================\n\tPress H to release the hand, F to release the foot\n================\n" << std::endl;

	// Load the gui
	app = std::unique_ptr<mcl::Application>( new mcl::Application( context->scene.get(), context.get() ) );
	app->settings.subdivide_meshes = true; // enable for better visuals
	app->zoom = 14.f;

	// Register a callback to draw the grabby spheres
	std::function<void ( GLFWwindow* window, RenderGL::AppCamera *cam, float screen_dt )> draw_cb(draw_callback);
	app->add_callback( draw_cb );

	// Callback to move around the grabby spheres
	using namespace std::placeholders;    // adds visibility of _1, _2, _3,...
	Input::key_callbacks.push_back( std::bind( &key_callback, _1, _2, _3, _4, _5 ) );
	Input::mouse_button_callbacks.clear(); // no moving the camera
	Input::mouse_button_callbacks.push_back( std::bind( &mouse_button_callback, _1, _2, _3, _4 ) );
	Input::cursor_position_callbacks.push_back( std::bind( &cursor_position_callback, _1, _2, _3 ) );

	// Render
	app->display();


	return 0;
}


void setup(){

	using namespace trimesh;

	hand.c = trimesh::point( .6, .8, .5 ); hand.rad = 0.2;
	foot.c = trimesh::point( -.25, -.6, -.1 ); foot.rad = 0.2;

	// Get the mesh. Those pointers tho...
	TriMesh *dillo = context->scene->objects_map[ "dillo" ]->get_TriMesh().get();

	std::vector< int > hand_ids, foot_ids;

	// Get the vertices that make up the hand a foot
	for( int i=0; i<dillo->vertices.size(); ++i ){
		trimesh::point p = dillo->vertices[i];
		if( hand.in_sphere( p ) ){ hand_ids.push_back( i ); }
		if( foot.in_sphere( p ) ){ foot_ids.push_back( i ); }
	}
	

	//
	//	Now add the forces
	//

	for( int i=0; i<hand_ids.size(); ++i ){
		int idx = hand_ids[i];
		std::shared_ptr<Force> tether_f( new MovingAnchor( idx, hand.control_points[i] ) );
		context->system->forces.push_back( tether_f );
	}

	for( int i=0; i<foot_ids.size(); ++i ){
		int idx = foot_ids[i];
		std::shared_ptr<Force> tether_f( new MovingAnchor( idx, foot.control_points[i] ) );
		context->system->forces.push_back( tether_f );
	}

	hand.make_end_points( trimesh::point( 2.6, .8, .5 ) );
	foot.make_end_points( trimesh::point( -2.25, -.6, -.1 ) );

	// Build the spheres for rendering
	mcl::Component c1( "object", "hand", "sphere" );
	c1.params.push_back( mcl::Param( "radius", "0.2" ) );
	c1.params.push_back( mcl::Param( "tess", "32" ) );
	mcl::Component c2( "object", "foot", "sphere" );
	c2.params.push_back( mcl::Param( "radius", "0.2" ) );
	c2.params.push_back( mcl::Param( "tess", "32" ) );
	hand.render_obj = default_build_object( c1 );
	foot.render_obj = default_build_object( c2 );


	trimesh::xform hand_xf = trimesh::xform::trans( hand.c );
	hand.render_obj->apply_xform( hand_xf );
	trimesh::xform foot_xf = trimesh::xform::trans( foot.c );
	foot.render_obj->apply_xform( foot_xf );

	ball_mat = context->scene->make_material( "blinnphong" );
	std::dynamic_pointer_cast<BlinnPhong>( ball_mat )->specular = trimesh::vec(0,0,0);
	std::dynamic_pointer_cast<BlinnPhong>( ball_mat )->diffuse = trimesh::vec(1,0,0);
}


void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ){

	if (action != GLFW_PRESS){ return; }

	switch( key ){

		case GLFW_KEY_H:
			activeHand = false; 
			for(int i = 0; i < hand.control_points.size(); i++){
				hand.control_points[i] -> active = !hand.control_points[i]->active;
				if( activeHand ){ hand.control_points[i]->anchorForce->weight = 500.f; }
				else{ hand.control_points[i]->anchorForce->weight = 0.f; }
			}
			context->system->recompute_weights();
		break;
		case GLFW_KEY_F:
			activeFoot = false;
			for(int i = 0; i < foot.control_points.size(); i++){
				foot.control_points[i] -> active = !foot.control_points[i]->active;
				if( activeFoot ){ foot.control_points[i]->anchorForce->weight = 500.f; }
				else{ foot.control_points[i]->anchorForce->weight = 0.f; }
			}
			context->system->recompute_weights();
		break;
	}
}


bool drag_active = false;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(  action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT ){
		drag_active = true;
	}
	else{	drag_active=false;
	}
}


int cursorX(0), cursorY(0);
float last_screen_dt;
void cursor_position_callback(GLFWwindow* window, double x, double y){

	glfwGetInputMode(window, GLFW_CURSOR);
	double alpha = double(x - cursorX) * 0.3;
	double beta = double(y - cursorY) * -0.3;
	cursorX = x;
	cursorY = y;

	if( drag_active && cursorX>0 && cursorY>0 ){

		float dt = last_screen_dt*scene_rad;
		Eigen::Vector3d move( dt*alpha, dt*beta, 0.f );

		if( activeHand ){
			for( int i=0; i<hand.control_points.size(); ++i ){
				hand.control_points[i]->pos += move;
			}
			hand.c += trimesh::vec( move[0], move[1], move[2] );

			trimesh::xform hand_xf = trimesh::xform::trans( move[0], move[1], move[2] );
			hand.render_obj->apply_xform( hand_xf );
			
		}
	}

}


void draw_callback( GLFWwindow* window, RenderGL::AppCamera *cam, float screen_dt ){

	if( activeHand ){ app->renderer.draw_mesh( hand.render_obj->get_TriMesh().get(), ball_mat, true ); }
	if( activeFoot ){ app->renderer.draw_mesh( foot.render_obj->get_TriMesh().get(), ball_mat, true ); }

	last_screen_dt = screen_dt;

} 



