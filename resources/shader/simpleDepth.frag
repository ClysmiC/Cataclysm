#version 330 core

out vec4 color; // debug
void main()
{
	gl_FragDepth = gl_FragCoord.z;
	color = vec4(vec3(gl_FragCoord.z), 1); // debug
}
