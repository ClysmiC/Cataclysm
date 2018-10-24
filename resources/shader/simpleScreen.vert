#version 330 core
layout (location = 0) in vec2 screenPos;
layout (location = 1) in vec2 texCoords;

out vec2 uv;

void main()
{
    gl_Position = vec4(screenPos.x, screenPos.y, 0.0, 1.0); 
    uv = texCoords;
}  
