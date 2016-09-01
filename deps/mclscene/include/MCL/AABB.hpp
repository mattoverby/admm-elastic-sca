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

#ifndef MCLSCENE_AABB_H
#define MCLSCENE_AABB_H 1

#include "Vec.h"

namespace mcl {

class AABB { // axis aligned bounding box
public:
	AABB() : valid(false) {}
	AABB( trimesh::vec min_, trimesh::vec max_ ) : min(min_), max(max_), valid(false) {}
	trimesh::vec min, max;
	bool valid;

	trimesh::vec center(){ return (min+max)*0.5f; }

	float radius(){ return trimesh::len(max-min)*0.5f; }

	AABB& operator+(const trimesh::vec& p){
		if( valid ){ min.min(p); max.max(p); }
		else{ min = p; max = p; }
		valid = true;
		return *this;
	}

	AABB& operator+=(const AABB& aabb){
		if( valid ){ min.min( aabb.min ); max.max( aabb.max ); }
		else{ min = aabb.min; max = aabb.max; }
		valid = true;
		return *this;
	}

	AABB& operator+=(const trimesh::vec& p){
		if( valid ){ min.min(p); max.max(p); }
		else{ min = p; max = p; }
		valid = true;
		return *this;
	}

};


} // end namespace mcl



#endif
