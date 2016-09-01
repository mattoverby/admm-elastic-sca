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

#ifndef MCLSCENE_PARAM_H
#define MCLSCENE_PARAM_H 1

#include <string>
#include <sstream>
#include "Vec.h"
#include "XForm.h"
#include <unordered_map>
#include "../../deps/pugixml/pugixml.hpp"

namespace mcl {

// Nice utility functions for parsing
namespace parse {

	static std::string to_lower( std::string s ){ std::transform( s.begin(), s.end(), s.begin(), ::tolower ); return s; }

	// Returns directory to a file
	static std::string fileDir( std::string fname ){
		size_t pos = fname.find_last_of('/');
		return (std::string::npos == pos) ? "" : fname.substr(0, pos)+'/';
	}

	// Returns file extention
	static std::string get_ext( std::string fname ){
		size_t pos = fname.find_last_of('.');
		return (std::string::npos == pos) ? "" : fname.substr(pos+1,fname.size());
	}

	// Returns file name without extention or directory
	// TODO some basic error checking
	static std::string get_fname( std::string fname ){
		std::string dir = fileDir( fname );
		std::string ext = get_ext( fname );
		return fname.substr( dir.size(), fname.size()-ext.size()-dir.size()-1 );
	}

	static std::string get_timestamp(){
		std::string MY_DATE_FORMAT = "h%H_m%M_s%S";
		const int MY_DATE_SIZE = 20;
		static char name[MY_DATE_SIZE];
		time_t now = time(0);
		strftime(name, sizeof(name), MY_DATE_FORMAT.c_str(), localtime(&now));
		return std::string(name);
	}
};


//
//	A parameter parsed from the scene file, stored as a string.
//	Has casting functions for convenience, but assumes the type
//	has an overloaded stream operator (with exception of trimesh::vec).
//	I'm really just copying what pugixml does.
//
//	Tag is ALWAYS lowercase
//
class Param {
public:
	Param( std::string tag_, std::string value_ ) : tag(tag_), value(value_) {}

	inline double as_double() const { std::stringstream ss(value); double v; ss>>v; return v; }
	inline char as_char() const { std::stringstream ss(value); char v; ss>>v; return v; }
	inline std::string as_string() const { return value; }
	inline int as_int() const { std::stringstream ss(value); int v; ss>>v; return v; }
	inline long as_long() const { std::stringstream ss(value); long v; ss>>v; return v; }
	inline bool as_bool() const { std::stringstream ss(value); bool v; ss>>v; return v; }
	inline float as_float() const { std::stringstream ss(value); float v; ss>>v; return v; }
	inline trimesh::vec4 as_vec4() const;
	inline trimesh::vec as_vec3() const;
	inline trimesh::vec2 as_vec2() const;
	inline trimesh::xform as_xform() const;

	// Stores the parsed data
	std::string tag;
	std::string value; // string value

	// Some useful vec3 functions:
	inline void normalize();
	inline void fix_color(); // if 0-255, sets 0-1
};


//
//	A component is basically a list of params.
//	Components are parsed from the xml file and always stored.
//
class Component {
public:
	Component( std::string tag_, std::string name_, std::string type_ ) : tag(tag_), name(name_), type(type_) {}
	std::string tag, name, type;
	inline Param &get( std::string tag );
	inline Param &operator[]( std::string tag ){ return get(tag); }
	inline bool exists( std::string tag ) const;
	std::vector<Param> params;
};


//
//	Parses the parameters from a pugi node and creates a vector of them.
//
static void load_params( std::vector<Param> &params, const pugi::xml_node &curr_node ){

	pugi::xml_node::iterator param = curr_node.begin();
	for( ; param != curr_node.end(); ++param ) {
		pugi::xml_node curr_param = *param;

		std::string tag = parse::to_lower( curr_param.name() );
		std::string value = curr_param.attribute("value").value();
		Param newP( tag, value );

		if( tag == "scale" ){
			std::stringstream ss( value );
			trimesh::vec v; ss >> v[0] >> v[1] >> v[2];
			trimesh::xform x_form = trimesh::xform::scale(v[0],v[1],v[2]);
			std::stringstream xf_ss; xf_ss << x_form;
			newP.value = xf_ss.str();
		}

		else if( tag == "translate" ){
			std::stringstream ss( value );
			trimesh::vec v; ss >> v[0] >> v[1] >> v[2];
			trimesh::xform x_form = trimesh::xform::trans(v[0],v[1],v[2]);
			std::stringstream xf_ss; xf_ss << x_form;
			newP.value = xf_ss.str();
		}

		else if( tag == "rotate" ){
			std::stringstream ss( value );
			trimesh::vec v; ss >> v[0] >> v[1] >> v[2];

				v *= (M_PI/180.f); // convert to radians
				trimesh::xform rot;
				rot = rot * trimesh::xform::rot( v[0], trimesh::vec(1.f,0.f,0.f) );
				rot = rot * trimesh::xform::rot( v[1], trimesh::vec(0.f,1.f,0.f) );
				rot = rot * trimesh::xform::rot( v[2], trimesh::vec(0.f,0.f,1.f) );
				trimesh::xform x_form = rot;

			std::stringstream xf_ss; xf_ss << x_form;
			newP.value = xf_ss.str();
		}

		params.push_back( newP );	

	} // end loop params

} // end load parameters


//
//	Implementation of Class functions
//


trimesh::vec Param::as_vec3() const {
	std::stringstream ss(value);
	trimesh::vec v;
	for( int i=0; i<3; ++i ){ ss>>v[i]; }
	return v;
}

trimesh::vec2 Param::as_vec2() const {
	std::stringstream ss(value);
	trimesh::vec2 v;
	for( int i=0; i<2; ++i ){ ss>>v[i]; }
	return v;
}

trimesh::vec4 Param::as_vec4() const {
	std::stringstream ss(value);
	trimesh::vec4 v;
	for( int i=0; i<4; ++i ){ ss>>v[i]; }
	return v;
}

trimesh::xform Param::as_xform() const {
	trimesh::xform x_form;
	std::stringstream ss(value);
	ss >> x_form;
	return x_form;
}

void Param::normalize(){

	// vec2, vec3, or vec4?
	std::stringstream sscheck( as_string() );
	int num_elem = 0;
	while( sscheck.good() ){ float buff; sscheck >> buff; num_elem++; }

	if(num_elem==3){
		trimesh::vec v = as_vec3();
		trimesh::normalize( v );
		std::stringstream ss; ss << v[0] << ' ' << v[1] << ' ' << v[2];
		value = ss.str();
	}
	else if(num_elem==2){
		trimesh::vec2 v = as_vec2();
		trimesh::normalize( v );
		std::stringstream ss; ss << v[0] << ' ' << v[1];
		value = ss.str();
	}
	else if(num_elem==4){
		trimesh::vec4 v = as_vec4();
		trimesh::normalize( v );
		std::stringstream ss; ss << v[0] << ' ' << v[1] << ' ' << v[2] << ' ' << v[3];
		value = ss.str();
	}

}

void Param::fix_color(){

	// vec2, vec3, or vec4?
	std::stringstream sscheck( as_string() );
	int num_elem = 0;
	while( sscheck.good() ){ float buff; sscheck >> buff; num_elem++; }

	if(num_elem==3){
		trimesh::vec c = as_vec3();

		for( int ci=0; ci<3; ++ci ){ if( c[ci]<0.f ){ c[ci]=0.f; } } // min zero
		if( c[0] > 1.0 || c[1] > 1.0 || c[2] > 1.0 ){ for( int ci=0; ci<3; ++ci ){ c[ci]/=255.f; } } // from 0-255 to 0-1

		std::stringstream ss; ss << c[0] << ' ' << c[1] << ' ' << c[2];
		value = ss.str();
	}
	else if(num_elem==2){
		trimesh::vec2 c = as_vec2();

		for( int ci=0; ci<2; ++ci ){ if( c[ci]<0.f ){ c[ci]=0.f; } } // min zero
		if( c[0] > 1.0 || c[1] > 1.0 ){ for( int ci=0; ci<2; ++ci ){ c[ci]/=255.f; } } // from 0-255 to 0-1

		std::stringstream ss; ss << c[0] << ' ' << c[1];
		value = ss.str();
	}
	else if(num_elem==4){
		trimesh::vec4 c = as_vec4();

		for( int ci=0; ci<4; ++ci ){ if( c[ci]<0.f ){ c[ci]=0.f; } } // min zero
		if( c[0] > 1.0 || c[1] > 1.0 || c[2] > 1.0 || c[3] > 1.0 ){ for( int ci=0; ci<4; ++ci ){ c[ci]/=255.f; } } // from 0-255 to 0-1

		std::stringstream ss; ss << c[0] << ' ' << c[1] << ' ' << c[2] << ' ' << c[3];
		value = ss.str();
	}

}


mcl::Param &Component::get( std::string tag ){
	for( int i=0; i<params.size(); ++i ){
		if( params[i].tag == tag ){ return params[i]; }
	}
	// not found, add it
	params.push_back( mcl::Param(tag,"") );
	return params.back();
}


bool Component::exists( std::string tag ) const {
	for( int i=0; i<params.size(); ++i ){
		if( params[i].tag == tag ){ return true; }
	}
	return false;
}


} // end namespace mcl

#endif
