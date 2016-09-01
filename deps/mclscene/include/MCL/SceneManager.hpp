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

#ifndef MCLSCENE_SCENEMANAGER_H
#define MCLSCENE_SCENEMANAGER_H 1

#include "BVH.hpp"
#include "DefaultBuilders.hpp"

//
//	Loading a scene with SceneManager:
//	1) Overwrite the build callbacks with your own methods, if needed.
//		See include/MCL/DefaultBuilders.hpp for details.
//	2) Load an XML file with SceneManager::load (see conf/ for examples).
//		OR you can use the make_object, make_light, make_camera, and make_material
//		functions. They return shared pointers of the scene component
//		and you can set the parameters directly.
//
namespace mcl {

class SceneManager {

	public:
		SceneManager();

		//
		// Load a configuration file, can be called multiple times for different files.
		// Additional calls will add (or replace, if same name) stuff to the scene.
		// Returns true on success
		//
		bool load( std::string filename );

		//
		// Exports to a scene file. Mesh files are saved to the build directory.
		// Note that some of the original scene file information will be lost
		// (i.e. names)
		// Mode is:
		//	0 = mclscene
		//
		void save( std::string xmlfile, int mode=0 );

		//
		// Computes bounding volume heirarchy (AABB). Eventually I will add better heuristics.
		// Type is:
		// 	spatial = object median (slower, better balanced)
		//	linear = parallel build w/ morton codes (probably has an error somewhere)
		//
		std::shared_ptr<BVHNode> get_bvh( bool recompute=false, std::string type="spatial" );

		//
		// For a given camera distance from scene center, add lights to make
		// a three-point lighting rig. Assumes +y is up and camera is facing -z.
		// Any previous lights are removed.
		// This is called by the Gui if no lighting has been added to the scene.
		//
		void make_3pt_lighting( trimesh::vec center, float distance );

		//
		// Returns the radius of the scene, excluding lights and cameras.
		// Calls each object's bound function, so may be costly for dynamic scenes.
		//
		float radius();

		//
		// Vectors and maps of scene components.
		// The shared pointers in the vectors and maps are duplicates, and exist twice for convenience.
		// Note that materials are only stored in the map as their reference name is used by objects.
		//
		std::vector< std::shared_ptr<BaseObject> > objects;
		std::vector< std::shared_ptr<BaseCamera> > cameras;
		std::vector< std::shared_ptr<BaseLight> > lights;
		std::unordered_map< std::string, std::shared_ptr<BaseObject> > objects_map; // name -> object
		std::unordered_map< std::string, std::shared_ptr<BaseMaterial> > materials_map; // name -> material
		std::unordered_map< std::string, std::shared_ptr<BaseCamera> > cameras_map; // name -> camera
		std::unordered_map< std::string, std::shared_ptr<BaseLight> > lights_map; // name -> light

		//
		// Creator functions that build a scene component and adds it to the
		// vectors and maps below. A default name is assigned (e.g. "obj1") if not given.
		// Calls the builder callbacks.
		//
		std::shared_ptr<BaseObject> make_object( std::string type, std::string name="" );
		std::shared_ptr<BaseLight> make_light( std::string type, std::string name="" );
		std::shared_ptr<BaseCamera> make_camera( std::string type, std::string name="" );
		std::shared_ptr<BaseMaterial> make_material( std::string type, std::string name="" );

		//
		// In addition to creating the components, the original parameters parsed
		// from the XML file (if any) are stored as vectors. They are listed in order parsed.
		//
		std::unordered_map< std::string, std::vector<Param> > object_params;
		std::unordered_map< std::string, std::vector<Param> > material_params;
		std::unordered_map< std::string, std::vector<Param> > camera_params;
		std::unordered_map< std::string, std::vector<Param> > light_params;

		//
		// Creator Callbacks, invoked on a "load" or "create_<thing>" call.
		// These can be changed to whatever. For more details, see include/MCL/DefaultBuilders.hpp
		//
		BuildObjCallback createObject;
		BuildCamCallback createCamera;
		BuildLightCallback createLight;
		BuildMatCallback createMaterial;

	protected:

		// Root bvh is created by build_bvh. split_mode assumed lower case
		void build_bvh( std::string split_mode );
		std::shared_ptr<BVHNode> root_bvh;

}; // end class SceneManager

} // end namespace mcl

#endif
