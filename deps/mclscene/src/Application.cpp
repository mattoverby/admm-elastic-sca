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

#include "SOIL2.h"
#include "MCL/Application.hpp"

using namespace mcl;

//
//	Initialize static members
//
std::vector< std::function<void ( GLFWwindow* window, int key, int scancode, int action, int mods )> > mcl::Input::key_callbacks;
std::vector< std::function<void ( GLFWwindow* window, int button, int action, int mods )> > mcl::Input::mouse_button_callbacks;
std::vector< std::function<void ( GLFWwindow* window, double x, double y )> > mcl::Input::cursor_position_callbacks;
std::vector< std::function<void ( GLFWwindow* window, double x, double y )> > mcl::Input::scroll_callbacks;
std::vector< std::function<void ( GLFWwindow* window, int width, int height )> > mcl::Input::framebuffer_size_callbacks;

//
// Application
//

Application::Application( mcl::SceneManager *scene_, Simulator *sim_ ) : scene(scene_), sim(sim_) {
	Input &input = Input::getInstance(); // initialize the singleton
	float scene_rad = scene->radius();
	trimesh::vec scene_center = scene->get_bvh()->aabb->center();
	if( scene->lights.size()==0 ){ scene->make_3pt_lighting( scene_center, scene_rad*6.f ); }

	std::cout << "Scene Radius: " << scene_rad << std::endl;

	aspect_ratio = 1.f;
	// real aspect_ratio set by framebuffer_size_callback

	zoom = std::fmaxf( fabs( scene_rad / sinf( 30.f/2.f ) ), 1e-3f );
	cursorX = 0.f;
	cursorY = 0.f;
	alpha = 0.f;
	beta = 0.f;

	// Add callbacks to the input class
	using namespace std::placeholders;    // adds visibility of _1, _2, _3,...
	Input::key_callbacks.push_back( std::bind( &Application::key_callback, this, _1, _2, _3, _4, _5 ) );
	Input::mouse_button_callbacks.push_back( std::bind( &Application::mouse_button_callback, this, _1, _2, _3, _4 ) );
	Input::cursor_position_callbacks.push_back( std::bind( &Application::cursor_position_callback, this, _1, _2, _3 ) );
	Input::scroll_callbacks.push_back( std::bind( &Application::scroll_callback, this, _1, _2, _3 ) );
	Input::framebuffer_size_callbacks.push_back( std::bind( &Application::framebuffer_size_callback, this, _1, _2, _3 ) );
}


Application::Application( mcl::SceneManager *scene_ ) : Application(scene_,0) {}

int Application::display(){

	GLFWwindow* window;
	glfwSetErrorCallback(&Input::error_callback);

	// Initialize the window
	if (!glfwInit()){ return false; }
	glfwWindowHint(GLFW_SAMPLES, 4); // anti aliasing
	glfwWindowHint(GLFW_SRGB_CAPABLE, true); // gamma correction

	// Get the monitor max window size
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int max_width = mode->width;
	int max_height = mode->height;
	if( max_width >= 1920 ){ max_width=1920; max_height=1080; } // just use 1080 if they have it
	else{ max_width=1366; max_height=768; } // any lower than this... why?

	// Create the glfw window
	window = glfwCreateWindow(max_width, max_height, "Viewer", NULL, NULL);
	if( !window ){ glfwTerminate(); return false; }

	// Bind callbacks to the window
	glfwSetKeyCallback(window, &Input::key_callback);
	glfwSetMouseButtonCallback(window, &Input::mouse_button_callback);
	glfwSetCursorPosCallback(window, &Input::cursor_position_callback);
	glfwSetScrollCallback(window, &Input::scroll_callback);
	glfwSetFramebufferSizeCallback(window, &Input::framebuffer_size_callback);

	// Make current
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewInit();
	if( !renderer.init( scene, &camera ) ){ return EXIT_FAILURE; } // creates shaders

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	framebuffer_size_callback(window, width, height); // sets the projection matrix

	// Initialize OpenGL
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// Game loop
	float t_old = glfwGetTime();
	screen_dt = 0.f;
	while( !glfwWindowShouldClose(window) ){

		//
		//	Update
		//

		float t = glfwGetTime();
		screen_dt = t - t_old;
		t_old = t;

		// Simulation engine:
		if( sim && settings.run_simulation ){
			if( !sim->step( scene, screen_dt ) ){ std::cerr << "\n**Application::display Error: Problem in simulation step" << std::endl; }
			if( !sim->update( scene ) ){ std::cerr << "\n**Application::display Error: Problem in mesh update" << std::endl; }

			// Recalculate normals for trimeshes and tetmeshes
			// Maybe make the simulator do this?
			for( int o=0; o<scene->objects.size(); ++o ){
				trimesh::TriMesh *themesh = scene->objects[o]->get_TriMesh().get();
				if( themesh==NULL ){ continue; }
				themesh->need_normals(true);
			}
		}

		//
		//	Render
		//

		{ // Clear screen
			glClearColor(settings.clear_color[0],settings.clear_color[1],settings.clear_color[2],1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if( settings.gamma_correction ){ glEnable(GL_FRAMEBUFFER_SRGB); } // gamma correction

			camera.model = trimesh::XForm<float>::rot( beta, trimesh::vec3(1.0f, 0.0f, 0.0f) ) *
				trimesh::XForm<float>::rot( alpha, trimesh::vec3(0.f, 0.f, 1.f) ) * trans;
			camera.view = trimesh::XForm<float>::trans( 0.0f, 0.0f, -zoom );
			camera.projection = trimesh::XForm<float>::persp( settings.fov_deg, aspect_ratio, settings.clipping[0], settings.clipping[1] );
		}

		{ // Render scene stuff
			if( !settings.subdivide_meshes ){ renderer.draw_objects(); } // draws all objects
			else{ renderer.draw_objects_subdivided(); }
			if( settings.draw_lights ){ renderer.draw_lights(); }
			for( int i=0; i<render_callbacks.size(); ++i ){ render_callbacks[i]( window, &camera, screen_dt ); }
		}

		{ // Finalize:
			glfwSwapBuffers(window);
			glfwPollEvents();
			if(settings.save_frames){ save_screenshot(window); }
		}

	} // end game loop

	return (EXIT_SUCCESS);

} // end display



void Application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

	if( action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT ){
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwGetCursorPos(window, &cursorX, &cursorY);
	}
	else if(  action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT ){
		// TODO
	}
	else{ glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

}



void Application::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS){ return; }

	switch (key)
	{
	case GLFW_KEY_ESCAPE:
	    glfwSetWindowShouldClose(window, true);
	    break;
	case GLFW_KEY_SPACE:
		settings.run_simulation = !settings.run_simulation;
		break;
	case GLFW_KEY_P:
		if( sim ){ sim->step( scene, screen_dt ); }
	    break;
	case GLFW_KEY_S:
		settings.save_frames=!settings.save_frames;
		std::cout << "save screenshots: " << (int)settings.save_frames << std::endl;
		break;
	default:
	    break;
	}
}


void Application::cursor_position_callback(GLFWwindow* window, double x, double y){

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED){
		alpha += (GLfloat) (x - cursorX) / 100.f;
		beta += (GLfloat) (y - cursorY) / 100.f;
		cursorX = x;
		cursorY = y;
	}
}


void Application::scroll_callback(GLFWwindow* window, double x, double y){
	float scene_rad = scene->radius();
	zoom -= float(y) * (scene_rad);
	if( zoom < 0.f ){ zoom=0.f; }
}


void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height){

	float scene_d = std::fmaxf( scene->radius()*2.f, 0.2f );
	aspect_ratio = 1.f;
	if( height > 0 ){ aspect_ratio = std::fmaxf( (float) width / (float) height, 1e-6f ); }
	glViewport(0, 0, width, height);

	camera.projection = trimesh::XForm<float>::persp( settings.fov_deg, aspect_ratio, 0.1f, scene_d*16.f );
}


void Application::save_screenshot(GLFWwindow* window){

	std::string MY_DATE_FORMAT = "h%H_m%M_s%S";
	const int MY_DATE_SIZE = 20;
	static char name[MY_DATE_SIZE];
	time_t now = time(0);
	strftime(name, sizeof(name), MY_DATE_FORMAT.c_str(), localtime(&now));

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);

	std::stringstream filename;
	filename << MCLSCENE_BUILD_DIR << "/screenshot_" << name << ".png";
	SOIL_save_screenshot( filename.str().c_str(), SOIL_SAVE_TYPE_PNG, 0, 0, w, h );

}

