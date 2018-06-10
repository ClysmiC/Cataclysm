#version 330 core

layout (location = 0) in vec3 position;

out vec3 texCoords;

uniform mat4 viewProjectionSansTranslation;

void main()
{
	texCoords = position;
	gl_Position = viewProjectionSansTranslation * vec4(position, 1.0);
}
