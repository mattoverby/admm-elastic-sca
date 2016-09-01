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

#ifndef MCLSCENE_CAMERA_H
#define MCLSCENE_CAMERA_H 1

#include <math.h>

namespace mcl {

//
//	Orthonormal Base
//
class OrthonormalBasis {
public:
	OrthonormalBasis( float *direction, float *up ){ init( direction, up ); }

	OrthonormalBasis( float *direction ){ float up[3]={0,1,0}; init( direction, up ); }

	float U[3], V[3], W[3];

private:
	inline void init( float *direction, float *up ){
		for( int i=0; i<3; ++i ){ W[i] = direction[i]*-1.f; }
		normalize( W );
		if( W[0] == 0.f && W[2] == 0.f ){
			up[0]+=0.0001;
			up[1]-=0.0001;
			up[2]+=0.0001;
			normalize( up );
		}
		cross( U, up, W );
		normalize( U );
		cross( V, W, U );
		normalize( V );
	}

	inline void cross( float *result, const float *v1, const float *v2 ){
		result[0] = v1[1]*v2[2] - v1[2]*v2[1];
		result[1] = v1[2]*v2[0] - v1[0]*v2[2];
		result[2] = v1[0]*v2[1] - v1[1]*v2[0];
	}
	inline void normalize( float *v ){
		float l = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
		if(l>0.f){ v[0]/=l; v[1]/=l; v[2]/=l; }
	}

};


//
//	Base, pure virtual
//
class BaseCamera {
public:
	virtual ~BaseCamera(){}


	// Returns a string containing xml code for saving to a scenefile.
	// Mode is:
	//	0 = mclscene
	//	1 = mitsuba
	virtual std::string get_xml( std::string camera_name, int mode ){ return ""; }
};


} // end namespace mcl

#endif
