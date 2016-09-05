
# admm-elastic-samples

[ADMM ⊇ Projective Dynamics: Fast Simulation of General Constitutive Models](http://rahul.narain.name/admm-pd/)  
[Rahul Narain](http://rahul.narain.name/), [Matthew Overby](http://www.mattoverby.net/), and [George E. Brown](http://www-users.cs.umn.edu/~brow2327/)  
University of Minnesota

Contains examples used in the above paper. The actual simulation code is included as a submodule
([admm-elastic](http://www.github.com/mattoverby/admm-elastic/)).

# citation

BibTex:

	@inproceedings{Narain2016,
	 author = {Narain, Rahul and Overby, Matthew and Brown, George E.},
	 title = {{ADMM} $\supseteq$ Projective Dynamics: Fast Simulation of General Constitutive Models},
	 booktitle = {Proceedings of the ACM SIGGRAPH/Eurographics Symposium on Computer Animation},
	 series = {SCA '16},
	 year = {2016},
	 isbn = {978-3-905674-61-3},
	 location = {Zurich, Switzerland},
	 pages = {21--28},
	 numpages = {8},
	 url = {http://dl.acm.org/citation.cfm?id=2982818.2982822},
	 acmid = {2982822},
	 publisher = {Eurographics Association},
	 address = {Aire-la-Ville, Switzerland, Switzerland},
	} 

# compliation

Requires OpenGL, GLEW, and GLFW3. Tested on Mac and linux. Before compiling, be sure to initialize the submodules:

	git submodule init
	git submodule update

# license

Copyright (c) 2016 University of Minnesota

ADMM-Elastic Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:  
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.  
2. Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.  
THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OF MINNESOTA, DULUTH OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
