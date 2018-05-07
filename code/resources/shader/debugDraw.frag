#version 330

uniform vec3 debugColor;
out vec4 color;

void main()
{
	color = vec4(debugColor, 1.0);
}
