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

#ifndef MCLSCENE_MATERIAL_H
#define MCLSCENE_MATERIAL_H 1

#include <memory>
#include <cassert>
#include "MCL/Param.hpp"

///
///	Simple object types
///
namespace mcl {

//
//	Base, pure virtual
//
class BaseMaterial {
public:
	BaseMaterial() : texture_file(""){}
	virtual ~BaseMaterial(){}

	virtual std::string get_type() const = 0;

	// Returns a string containing xml code for saving to a scenefile.
	// Mode is:
	//	0 = mclscene
	//	1 = mitsuba
	virtual std::string get_xml( std::string material_name, int mode ){ return ""; }

	std::string texture_file;
};


//
//	BlinnPhong Material
//
class BlinnPhong : public BaseMaterial {
public:
	BlinnPhong() : ambient(.5,.5,.5), diffuse(.5,.5,.5), specular(0,0,0), shininess(0) {}
	BlinnPhong( trimesh::vec amb, trimesh::vec diff, trimesh::vec spec, float shini ) : ambient(amb), diffuse(diff), specular(spec), shininess(shini) {}

	trimesh::vec ambient;
	trimesh::vec diffuse;
	trimesh::vec specular;
	float shininess;

	std::string get_type() const { return "blinnphong"; }

	std::string get_xml( std::string material_name, int mode ){

		// mclscene
		if( mode == 0 ){
			std::stringstream xml;
			xml << "\t<Material name=\"" << material_name << "\" type=\"blinnphong\" >\n";
			xml << "\t\t<Ambient value=\"" << ambient.str() << "\" />\n";
			xml << "\t\t<Diffuse value=\"" << diffuse.str() << "\" />\n";
			xml << "\t\t<Specular value=\"" << specular.str() << "\" />\n";
			xml << "\t\t<Shininess  value=\"" << shininess << "\" />\n";
			if( texture_file.size() ){ xml << "\t\t<texture value=\"" << texture_file << "\" />\n"; }
			xml << "\t</Material>";
			return xml.str();
		}

		return "";

	} // end get xml
};

//
//	OpenGL Material Presets
//	See: http://devernay.free.fr/cours/opengl/materials.html
//
enum class MaterialPreset {
	Emerald, Jade, Obsidian, Pearl, Ruby, Turquoise, // gems
	Brass, Bronze, Chrome, Copper, Gold, Silver, // metals
	BlackPlastic, CyanPlastic, GreenPlastic, RedPastic, WhitePlastic, YellowPlastic, // plastic
	BlackRubber, CyanRubber, GreenRubber, RedRubber, WhiteRubber, YellowRubber, // rubber
	Unknown
};

static MaterialPreset material_str_to_preset( std::string preset ){
	preset = parse::to_lower(preset);

	// Gemstones
	if( preset=="emerald"){ return ( MaterialPreset::Emerald ); }
	else if( preset=="jade"){ return ( MaterialPreset::Jade ); }
	else if( preset=="obsidian"){ return ( MaterialPreset::Obsidian ); }
	else if( preset=="pearl"){ return ( MaterialPreset::Pearl ); }
	else if( preset=="ruby"){ return ( MaterialPreset::Ruby ); }
	else if( preset=="turquoise"){ return ( MaterialPreset::Turquoise ); }

	// Metals
	else if( preset=="brass"){ return ( MaterialPreset::Brass ); }
	else if( preset=="bronze"){ return ( MaterialPreset::Bronze ); }
	else if( preset=="chrome"){ return ( MaterialPreset::Chrome ); }
	else if( preset=="copper"){ return ( MaterialPreset::Copper ); }
	else if( preset=="gold"){ return ( MaterialPreset::Gold ); }
	else if( preset=="silver"){ return ( MaterialPreset::Silver ); }

	// Plastics
	else if( preset=="blackplastic"){ return ( MaterialPreset::BlackPlastic ); }
	else if( preset=="cyanplastic"){ return ( MaterialPreset::CyanPlastic ); }
	else if( preset=="greenplastic"){ return ( MaterialPreset::GreenPlastic ); }
	else if( preset=="redpastic"){ return ( MaterialPreset::RedPastic ); }
	else if( preset=="whiteplastic"){ return ( MaterialPreset::WhitePlastic ); }
	else if( preset=="yellowplastic"){ return ( MaterialPreset::YellowPlastic ); }

	// Rubber
	else if( preset=="blackrubber"){ return ( MaterialPreset::BlackRubber ); }
	else if( preset=="cyanrubber"){ return ( MaterialPreset::CyanRubber ); }
	else if( preset=="greenrubber"){ return ( MaterialPreset::GreenRubber ); }
	else if( preset=="redrubber"){ return ( MaterialPreset::RedRubber ); }
	else if( preset=="whiterubber"){ return ( MaterialPreset::WhiteRubber ); }
	else if( preset=="yellowrubber"){ return ( MaterialPreset::YellowRubber ); }

	return MaterialPreset::Unknown;
}

static std::shared_ptr<BlinnPhong> make_preset_material( MaterialPreset m ){
	using namespace trimesh;
	std::shared_ptr<BlinnPhong> r = NULL;
	switch( m ){

	// Gemstones
	case MaterialPreset::Emerald:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0215, 0.1745, 0.0215), vec(0.07568, 0.61424, 0.07568), vec(0.633, 0.727811, 0.633), 0.6 ) ); break;
	case MaterialPreset::Jade:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.135, 0.2225, 0.1575), vec(0.54, 0.89, 0.63), vec(0.316228, 0.316228, 0.316228), 0.1 ) ); break;
	case MaterialPreset::Obsidian:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.05375, 0.05, 0.06625), vec(0.18275, 0.17, 0.22525), vec(0.332741, 0.328634, 0.346435), 0.3 ) ); break;
	case MaterialPreset::Pearl:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.25, 0.20725, 0.20725), vec(1.0, 0.829, 0.829), vec(0.296648, 0.296648, 0.296648), 0.088 ) ); break;
	case MaterialPreset::Ruby:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.1745, 0.01175, 0.01175), vec(0.61424, 0.04136, 0.04136), vec(0.727811, 0.626959, 0.626959), 0.6 ) ); break;
	case MaterialPreset::Turquoise:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.1, 0.18725, 0.1745), vec(0.396, 0.74151, 0.69102), vec(0.297254, 0.30829, 0.306678), 0.1 ) ); break;

	// Metals
	case MaterialPreset::Brass:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.329412, 0.223529, 0.027451), vec(0.780392, 0.568627, 0.113725), vec(0.992157, 0.941176, 0.807843), 0.21794872 ) ); break;
	case MaterialPreset::Bronze:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.2125, 0.1275, 0.054), vec(0.714, 0.4284, 0.18144), vec(0.393548, 0.271906, 0.166721), 0.2 ) ); break;
	case MaterialPreset::Chrome:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.25, 0.25, 0.25), vec(0.4, 0.4, 0.4), vec(0.774597, 0.774597, 0.774597), 0.6 ) ); break;
	case MaterialPreset::Copper:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.19125, 0.0735, 0.0225), vec(0.7038, 0.27048, 0.0828), vec(0.256777, 0.137622, 0.086014), 0.6 ) ); break;
	case MaterialPreset::Gold:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.24725, 0.1995, 0.0745), vec(0.75164, 0.60648, 0.22648), vec(0.628281, 0.555802, 0.366065), 0.4 ) ); break;
	case MaterialPreset::Silver:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.19225, 0.19225, 0.19225), vec(0.50754, 0.50754, 0.50754), vec(0.508273, 0.508273, 0.508273), 0.4 ) ); break;

	// Plastics
	case MaterialPreset::BlackPlastic:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.0, 0.0), vec(0.01, 0.01, 0.01), vec(0.50, 0.50, 0.50), 0.25 ) ); break;
	case MaterialPreset::CyanPlastic:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.1, 0.06), vec(0.0, 0.50980392, 0.50980392), vec(0.50196078, 0.50196078, 0.50196078), 0.25 ) ); break;
	case MaterialPreset::GreenPlastic:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.0, 0.0), vec(0.1, 0.35, 0.1), vec(0.45, 0.55, 0.45), 0.25 ) ); break;
	case MaterialPreset::RedPastic:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.0, 0.0), vec(0.5, 0.0, 0.0), vec(0.7, 0.6, 0.6), 0.25 ) ); break;
	case MaterialPreset::WhitePlastic:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.0, 0.0), vec(0.55, 0.55, 0.55), vec(0.70, 0.70, 0.70), 0.25 ) ); break;
	case MaterialPreset::YellowPlastic:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.0, 0.0), vec(0.5, 0.5, 0.0), vec(0.60, 0.60, 0.50), 0.25 ) ); break;

	// Rubbers
	case MaterialPreset::BlackRubber:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.02, 0.02, 0.02), vec(0.01, 0.01, 0.01), vec(0.4, 0.4, 0.4), 0.078125 ) ); break;
	case MaterialPreset::CyanRubber:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.05, 0.05), vec(0.4, 0.5, 0.5), vec(0.04, 0.7, 0.7), 0.078125 ) ); break;
	case MaterialPreset::GreenRubber:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.0, 0.05, 0.0), vec(0.4, 0.5, 0.4), vec(0.04, 0.7, 0.04), 0.078125 ) ); break;
	case MaterialPreset::RedRubber:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.05, 0.0, 0.0), vec(0.5, 0.4, 0.4), vec(0.7, 0.04, 0.04), 0.078125 ) ); break;
	case MaterialPreset::WhiteRubber:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.05, 0.05, 0.05), vec(0.5, 0.5, 0.5), vec(0.7, 0.7, 0.7), 0.078125 ) ); break;
	case MaterialPreset::YellowRubber:
		r= std::shared_ptr<BlinnPhong>( new BlinnPhong( vec(0.05, 0.05, 0.0), vec(0.5, 0.5, 0.4), vec(0.7, 0.7, 0.04), 0.078125 ) ); break;

	default: break;

	} // end switch preset

	if( r != NULL ){ r->shininess *= 128.f; }
	else{ std::cerr << "**make_preset_material Error: Unknown material preset" << std::endl; }
	return r;

} // end material preset


} // end namespace mcl

#endif
