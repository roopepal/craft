#version 120

attribute vec4 a_coord;
uniform mat4 u_mvp;
varying vec4 texcoord;

void main()
{
	texcoord = a_coord;
	gl_Position = u_mvp * vec4(a_coord.xyz, 1);
}