#version 120
varying vec3 Normal;
varying vec3 FragPos;
varying vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int hastex;

void main()
{
	gl_Position = projection * view * model * gl_Vertex;
	Normal = gl_Normal;
	FragPos = vec3( gl_Vertex );
	TexCoord = vec2(0,0);
	if( hastex>0 ){ TexCoord = vec2( gl_MultiTexCoord0 ); }
	gl_PointSize = 8.f;
}
