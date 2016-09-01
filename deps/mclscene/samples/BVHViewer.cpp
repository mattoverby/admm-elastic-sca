
#include "MCL/SceneManager.hpp"
#include "MCL/Application.hpp"

using namespace mcl;

SceneManager scene;
void render_callback(GLFWwindow* window, RenderGL::AppCamera *cam, float screen_dt);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
std::vector< bool > traversal; // 0 = left, 1 = right
bool view_all = false;
std::string bvh_mode = "linear";
std::vector<trimesh::point> edges;

int main(int argc, char *argv[]){

	if( argc < 2 ){ printf("Usage: %s <config file>\n", argv[0]); return 0; }

	if( !scene.load( std::string(argv[1]) ) ){ return 0; }
	else{ printf( "Successfully loaded xml file.\n"); }

	// Build the initial bvh
	scene.get_bvh(true,bvh_mode);

	Application app( &scene );

	// Add a render callback to draw the BVH
	std::function<void ( GLFWwindow* window, RenderGL::AppCamera *cam, float screen_dt )> draw_cb(render_callback);
	app.add_callback( draw_cb );

	// Add a key callback to change the viewer settings
	std::function<void ( GLFWwindow* window, int key, int scancode, int action, int mods )> key_cb(key_callback);
	Input::key_callbacks.push_back( key_cb );

	app.display();

	return 0;
}



void render_callback(GLFWwindow* window, RenderGL::AppCamera *cam, float screen_dt){

	std::shared_ptr<BVHNode> bvh = scene.get_bvh();
	if( !view_all ){
		edges.clear();
		for( int i=0; i<traversal.size(); ++i ){
			bool right = traversal[i];
			if( right ){
				if( bvh->right_child != NULL ){ bvh = bvh->right_child; }
				else{ traversal.pop_back(); }
			} else {
				if( bvh->left_child != NULL ){ bvh = bvh->left_child; }
				else{ traversal.pop_back(); }
			}
		}
	
//		bvh->aabb->get_edges( edges );
	} else if( edges.size()<=24 ) {
		bvh->get_edges( edges );
	}

	trimesh::XForm<float> xf = cam->projection * cam->view * cam->model;

//		glLineWidth(10.f);
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	for( int j=0; j<edges.size(); j+=2 ){
//		glMultMatrixf(xform);
		trimesh::vec e1 = xf*edges[j];
		trimesh::vec e2 = xf*edges[j+1];
		glVertex3f(e1[0],e1[1],e1[2]);
		glVertex3f(e2[0],e2[1],e2[2]);
	}
	glEnd();
	glEnable(GL_LIGHTING);

}


void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ){

	if (action != GLFW_PRESS){ return; }

	switch(key){

	case GLFW_KEY_M:
		if( bvh_mode=="spatial" ){ bvh_mode="linear"; }
		else{ bvh_mode = "spatial"; }
		edges.clear();
		scene.get_bvh(true,bvh_mode);
		std::cout << "BVH Mode: " << bvh_mode << std::endl;
		break;
	case GLFW_KEY_DOWN:
		view_all = !view_all;
		break;
	}


/*
	if( event.type == sf::Event::KeyPressed ){

		if( event.key.code == sf::Keyboard::Up && traversal.size() ){ traversal.pop_back(); }
		else if( event.key.code == sf::Keyboard::Left ){ traversal.push_back( false ); }
		else if( event.key.code == sf::Keyboard::Right ){ traversal.push_back( true ); }
		else if( event.key.code == sf::Keyboard::R ){ refine_mesh("-"); }
		else if( event.key.code == sf::Keyboard::Numpad6 ){ scale_mesh("+x"); }
		else if( event.key.code == sf::Keyboard::Numpad4 ){ scale_mesh("-x"); }
		else if( event.key.code == sf::Keyboard::Numpad8 ){ scale_mesh("+y"); }
		else if( event.key.code == sf::Keyboard::Numpad2 ){ scale_mesh("-y"); }
	}
*/
}


