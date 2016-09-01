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

#ifndef MCLSCENE_BVH_H
#define MCLSCENE_BVH_H 1

#include "Object.hpp"
#include "AABB.hpp"
#include <memory>
#include <numeric>

namespace mcl {

typedef long morton_type;
typedef unsigned long long morton_encode_type;

class BVHNode {
public:
	BVHNode() : aabb( new AABB ) { left_child=NULL; right_child=NULL; }

	// Allocated in make_tree:
	std::shared_ptr<BVHNode> left_child;
	std::shared_ptr<BVHNode> right_child;
	std::shared_ptr<AABB> aabb;
	std::vector< std::shared_ptr<BaseObject> > m_objects; // empty unless a leaf node

	bool is_leaf() const { return m_objects.size()>0; }
	void get_edges( std::vector<trimesh::vec> &edges ); // for visual debugging
	void bounds( trimesh::vec &bmin, trimesh::vec &bmax ){ bmin=aabb->min; bmax=aabb->max; }
};


class BVHBuilder {
public:
	// Parallel sorting construction (Lauterbach et al. 2009)
	// returns num nodes in tree
	static int make_tree_lbvh( std::shared_ptr<BVHNode> &root, const std::vector< std::shared_ptr<BaseObject> > &objects, int max_depth=10 );

	// Object Median split, round robin axis
	// returns num nodes in tree
	static int make_tree_spatial( std::shared_ptr<BVHNode> &root, const std::vector< std::shared_ptr<BaseObject> > &objects, int max_depth=10 );

	// Stats used for profiling:
	static int n_nodes; // number of nodes in the last-created tree
	static float avg_balance; // the "balance" of the last-created tree (lousy metric, but whatever)
	static float runtime_s; // time it took to build the bvh (seconds)

private:
	static void lbvh_split( std::shared_ptr<BVHNode> &node, const int bit, const std::vector< std::shared_ptr<BaseObject> > &prims,
		const std::vector< std::pair< morton_type, int > > &morton_codes, const int max_depth );
	static void spatial_split( std::shared_ptr<BVHNode> &node, const std::vector< std::shared_ptr<BaseObject> > &objects,
		const std::vector< int > &queue, const int split_axis, const int max_depth );

	static int num_avg_balance;
};


class BVHTraversal {
public:
	// Ray-Scene traversal for closest object (light rays)
	// Can also be used for selection rays if the last argument is used, as it sets the shared ptr of the object hit.
	static bool closest_hit( const std::shared_ptr<BVHNode> node, const intersect::Ray *ray, intersect::Payload *payload, std::shared_ptr<BaseObject> *obj=0 );

	// Ray-Scene traversal for any object, early exit (shadow rays)
	static bool any_hit( const std::shared_ptr<BVHNode> node, const intersect::Ray *ray, intersect::Payload *payload );
};


} // end namespace mcl



#endif
