// Copyright 2016 Matthew Overby.
// 
// MCLTOOLS Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
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


#ifndef MCLTOOLS_ARGPARSER_H
#define MCLTOOLS_ARGPARSER_H 1

#include <sstream>
#include <string>
#include <vector>
#include <map>


//
// MCLTOOLS ArgParser does simple space-delimited argument parsing
//
// What it can do:
//	-Any argument type (so long as >> operator is defined, see test_ArgParser.cpp)
//	-Out of order argument retrieval (see example below)
//	-Multi-value argument (space delimited input, comma delimited output,
//		see testString argument in example below)
//
// What it doesn't do:
//	-Error correction or detection
//
// To run a sample program:
//
//	./test_parser --testDouble 0.3 --testString hello world --testInt 3 --testFloat 0.33
//
// To load your arguments in the source:
//
//	mcl::ArgParser aParser( argc, argv );
//	double testDouble = aParser.get<double>("testDouble");
//	float testFloat = aParser.get<float>("testFloat");
//	int testInt = aParser.get<int>("testInt");
//	std::string testString = aParser.get<std::string>("testString");
//
// Or if you have defaults that you want to overwrite if the argument is given:
//
//	double overwriteMe = 3.0;
//	aParser.get<double>("overwriteMe", &overwriteMe);
//
// And each value will be:
//
//	testDouble: 0.3
//	testFloat: 0.33
//	testInt: 3
//	testString: hello,world
//


namespace mcl {


class ArgParser {

	public:
		ArgParser( const int &argc, char** argv ) {
			std::vector<std::string> args_strings( argc-1, "" );
			for( int i=1; i<argc; i++ ){
				std::stringstream inputSS( argv[i] );
				inputSS >> args_strings[i-1];
			}
			formTable( args_strings );
		} // end constructor

		template< typename T > const T get( const std::string label ) const {
			if( args.count( label ) > 0 ){
				std::stringstream ss( args.at(label) );
				T result; ss >> result; return result;
			}
			else{ return T(); }
		} // end getter

		// Return true on exists, false otherwise
		template< typename T > const bool get( const std::string label, T *result ) const {
			if( exists( label ) ){
				std::stringstream ss( args.at(label) );
				ss >> *result; return true;
			}
			else{ return false; }
		} // end getter to reference

		const bool exists( const std::string label ) const {
			if( args.count( label ) > 0 ){ return true; }
			return false;
		}

	protected:
		std::map< std::string, std::string > args; // [ arg, value ]

		const void formTable( const std::vector<std::string> &args_strings ) {
			std::string curr_label = "";
			for( int i=0; i<args_strings.size(); i++ ){
				std::string curr = args_strings[i];
				size_t index = curr.find("--");
				if( index == std::string::npos && curr_label.size() > 0 ){
					std::string curr_val( args[curr_label] );
					std::stringstream ss; ss << curr_val;
					if( ss.str().size() > 0 ){ ss << ","; }
					ss << curr; args[curr_label] = ss.str();
				}
				else{
					curr_label = curr.substr(2);
					args[curr_label] = std::string("");
				}
			} // end loop strings
		} // end form table

}; // end class ArgParser


}; // end namespace mcl

#endif
