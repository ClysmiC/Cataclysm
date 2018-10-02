#version 330

layout (location = 0) in vec3 position;

uniform mat4 viewProjectionMatrix;
uniform mat4 model;

void main()
{
    gl_Position = viewProjectionMatrix * model * vec4(position, 1.0);
}
