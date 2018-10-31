#version 330 core
out vec4 color;
  
in vec2 uv;

uniform sampler2D tex;

void main()
{
    color = vec4(vec3(texture(tex, uv).r), 1);
}
