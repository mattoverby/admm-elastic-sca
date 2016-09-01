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

#ifndef MCLSCENE_RAYINTERSECT_H
#define MCLSCENE_RAYINTERSECT_H 1

#include <memory>
#include <Vec.h>

namespace mcl {

namespace intersect {

	class Ray {
	public:
		Ray(){ direction=trimesh::vec(0,0,-1); eps=1e-6f; }
		Ray( trimesh::vec o, trimesh::vec d, float e=1e-6f ){
			origin=o; direction=d; eps=e;
			origin += trimesh::unit(direction)*eps;
		}
		trimesh::vec origin, direction;
		float eps;
	};

	class Payload {
	public:
		Payload( const Ray *ray ){ t_min=ray->eps; t_max=9999999.0; launch_point=ray->origin; }
		double t_min, t_max;
		trimesh::vec n, hit_point, launch_point;
		std::string material;
	};

	static inline trimesh::vec reflect( const trimesh::vec incident, const trimesh::vec norm ){
		return ( incident - 2.f * norm * norm.dot( incident ) );
	}

	// ray -> triangle without early exit
	static inline bool ray_triangle( const Ray *ray, const trimesh::vec &p0, const trimesh::vec &p1, const trimesh::vec &p2,
		const trimesh::vec &n0, const trimesh::vec &n1, const trimesh::vec &n2, Payload *payload );

	// ray -> axis aligned bounding box
	// Returns true/false only and does not set the payload.
	static inline bool ray_aabb( const Ray *ray, const trimesh::vec &min, const trimesh::vec &max, const Payload *payload );

} // end namespace intersect

} // end namespace mcl

//
//	Implementation below
//

// ray -> triangle without early exit
static inline bool mcl::intersect::ray_triangle( const Ray *ray, const trimesh::vec &p0, const trimesh::vec &p1, const trimesh::vec &p2,
	const trimesh::vec &n0, const trimesh::vec &n1, const trimesh::vec &n2, Payload *payload ){
	using namespace trimesh;

	const vec e0 = p1 - p0;
	const vec e1 = p0 - p2;
	const vec n = e1.cross( e0 );

	const vec e2 = ( 1.0f / n.dot( ray->direction ) ) * ( p0 - ray->origin );
	const vec i  = ray->direction.cross( e2 );

	float beta  = i.dot( e1 );
	float gamma = i.dot( e0 );
	float alpha = 1.f - beta - gamma;

	float t = n.dot( e2 );
	bool hit = ( (t<payload->t_max) & (t>payload->t_min) & (beta>=-ray->eps*0.5f) & (gamma>=-ray->eps*0.5f) & (beta+gamma<=1.f) );

	if( hit ){
		payload->n = alpha*n0 + beta*n1 + gamma*n2;
		payload->t_max = t;
		payload->hit_point = ray->origin + ray->direction*t;
		return true;
	}

	return false;

} // end  ray -> triangle

// ray -> axis aligned bounding box
static inline bool mcl::intersect::ray_aabb( const Ray *ray, const trimesh::vec &min, const trimesh::vec &max, const Payload *payload ){

	float txmin=0.f, txmax=0.f;
	float dirX = 1.f / ray->direction[0];
	if( dirX >= 0.0 ){
		txmin = dirX * ( min[0] - ray->origin[0] );
		txmax = dirX * ( max[0] - ray->origin[0] );
	}
	else{
		txmax = dirX * ( min[0] - ray->origin[0] );
		txmin = dirX * ( max[0] - ray->origin[0] );
	}

	float tymin=0.f, tymax=0.f;
	float dirY = 1.f / ray->direction[1];
	if( ray->direction[1] >= 0.0 ){
		tymin = dirY * ( min[1] - ray->origin[1] );
		tymax = dirY * ( max[1] - ray->origin[1] );
	}
	else{
		tymax = dirY * ( min[1] - ray->origin[1] );
		tymin = dirY * ( max[1] - ray->origin[1] );
	}

	// First check: x/y axis
	if( txmin > tymax || tymin > txmax ){ return false; }

	float tzmin=0.f, tzmax=0.f;
	float dirZ = 1.f / ray->direction[2];
	if( ray->direction[2] >= 0.0 ){
		tzmin = dirZ * ( min[2] - ray->origin[2] );
		tzmax = dirZ * ( max[2] - ray->origin[2] );
	}
	else{
		tzmax = dirZ * ( min[2] - ray->origin[2] );
		tzmin = dirZ * ( max[2] - ray->origin[2] );
	}

	// Second check: z axis
	if( txmin > tzmax || tzmin > txmax ){ return false; }
	if( tymin > tzmax || tzmin > tymax ){ return false; }

	return true;

} // end ray box intersection

#endif
