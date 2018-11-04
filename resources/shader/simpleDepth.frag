#version 330 core

uniform int windowWidth;
uniform int windowHeight;

void main()
{
    if (gl_FragCoord.x <= 1 || gl_FragCoord.y <= 1 || gl_FragCoord.x >= windowWidth - 1 || gl_FragCoord.y >= windowHeight - 1) gl_FragDepth = 1;
    else gl_FragDepth = gl_FragCoord.z;
}
