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

#ifndef ADMM_CONTEXT_H
#define ADMM_CONTEXT_H 1

#include "MCL/Simulator.hpp"
#include "System.hpp" // admm-elastic
#include "ForceBuilder.hpp"

class SimContext : public mcl::Simulator {
public:
	// SolverSettings are set by the XML file.
	// You can also manually change these after the scene file is loaded.
	struct Settings {
		bool run_realtime; // <realtime value="1" />
		Settings() : run_realtime(false) {}
	} settings;

	// Public context data
	std::shared_ptr<admm::System> system;
	std::shared_ptr<mcl::SceneManager> scene;

	// SimContext constructor creates scene and system,
	// as well as sets up the ForceBuilder.
	SimContext();
	~SimContext(){ admm::ForceBuilder::reset(); }

	// Load one or more configuration/scene files. Scene elements
	// and forces will be added (or overwritten) with each call.
	// Will throw a runtime error on a failure.
	void load( std::string config_file );

	// Once you're done loading files, call initialize. This
	// will throw a runtime error on a failure.
	void initialize();

	// Step is called by the application gui with a key press (P)
	// or every frame (spacebar).
	bool step( const mcl::SceneManager *scene_, float screen_dt );

	// Update is called by the application gui to update the render meshes.
	// This means we have a copy of every mesh, unfortunately.
	bool update( mcl::SceneManager *scene_ );

private:
	// Handy index hash for mapping system node indices to mesh-vertex indices.
	// Only dynamic meshes are added to this map.
	std::unordered_map< int, std::pair< int, int > > system_to_scene_map;

	// Map table for parsing force values (e.g. stiffness) from the XML file.
	// It's used by ForceBuilder to set values.
	std::unordered_map< std::string, mcl::Component > force_param_map;

}; // end class SimContext

#endif

