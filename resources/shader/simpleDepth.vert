#version 330

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 lightMatrix;

void main()
{
    gl_Position = lightMatrix * model * vec4(position, 1.0);
}
