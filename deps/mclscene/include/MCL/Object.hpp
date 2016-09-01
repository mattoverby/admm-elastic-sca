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

#ifndef MCLSCENE_OBJECT_H
#define MCLSCENE_OBJECT_H 1

#include <memory>
#include "XForm.h"
#include "TriMesh.h"
#include "RayIntersect.hpp"

///
///	Object Primitives
///
namespace mcl {

//
//	Base, pure virtual
//
class BaseObject : public std::enable_shared_from_this<BaseObject> {
public:
	virtual ~BaseObject(){}
	virtual std::string get_type() const = 0;
	virtual void bounds( trimesh::vec &bmin, trimesh::vec &bmax ) = 0;

	// When an object's physical parameters are changed, it may need to
	// update its bounding box, internal parameters, etc...
	virtual void update(){}

	virtual const std::shared_ptr<trimesh::TriMesh> get_TriMesh(){ return NULL; }
	virtual void apply_xform( const trimesh::xform &xf ){} // TODO store xform

	virtual std::string get_material() const { return ""; }
	virtual void set_material( std::string mat ) = 0;

	// Used by BVHTraversal
	virtual bool ray_intersect( const intersect::Ray *ray, intersect::Payload *payload ) const { return false; }

	// Returns a string containing xml code for saving to a scenefile.
	virtual std::string get_xml( std::string component_name, int mode=0 ){ return ""; }

	// If an object is made up of other (smaller) objects, they are needed for BVH construction
	virtual void get_primitives( std::vector< std::shared_ptr<BaseObject> > &prims ){ prims.push_back( shared_from_this() ); }
};


} // end namespace mcl

#endif
