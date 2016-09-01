# mclscene

By Matt Overby  
[http://www.mattoverby.net](http://www.mattoverby.net)

mclscene is a 3D scene loader/manager for prototyping my graphics projects (e.g., physics simulation, ray tracing). It parses an XML scene file and creates typical world components (meshes, lights, materials).
Included are distributions of:
trimesh2 (Szymon Rusinkiewicz, gfx.cs.princeton.edu/proj/trimesh2),
tetgen (Hang Si, wias-berlin.de/software/tetgen),
pugixml (Arseny Kapoulkine, pugixml.org), and
soil (Jonathan Dummer, lonesock.net/soil.html).

There is a viewer which can render the scene using OpenGL, GLEW, and GLFW3.

<img src="https://github.com/over0219/mclscene/raw/master/doc/dillo.png" width="512">

# linking

The best way to include mclscene in your project is to add it as a submodule with git,
use cmake to call add_subdirectory, then use the ${MCLSCENE_LIBRARIES} and
${MCLSCENE_INCLUDE_DIRS} that are set during the build. See CMakeLists.txt for more details.
I will improve this in the future with a proper FindMCLSCENE.cmake.

# materials

There are several usable materials with [preset Blinn-Phong parameters](http://devernay.free.fr/cours/opengl/materials.html).
To reference them in the scene file use the following names:
Emerald, Jade, Obsidian, Pearl, Ruby, Turquoise,
Brass, Bronze, Chrome, Copper, Gold, Silver,
BlackPlastic, CyanPlastic, GreenPlastic, RedPastic, WhitePlastic, YellowPlastic,
BlackRubber, CyanRubber, GreenRubber, RedRubber, WhiteRubber, and YellowRubber.

# todo

- Cleanup headers and make it header-only
- Cleanup object base class and hierarchy
- Support for texture wrapping
- Support for parsing custom scene components
- Mesh instancing
- Export to other renderers (e.g., mitsuba, blender)

# license

Copyright 2016 Matthew Overby.

MCLSCENE Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
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

By Matt Overby (http://www.mattoverby.net)
