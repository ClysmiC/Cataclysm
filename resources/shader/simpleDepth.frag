#version 330 core

uniform int viewportWidth;
uniform int viewportHeight;

void main()
{
    if (gl_FragCoord.x <= 1 || gl_FragCoord.y <= 1 || gl_FragCoord.x >= viewportWidth - 1 || gl_FragCoord.y >= viewportHeight - 1) gl_FragDepth = 1;
    else gl_FragDepth = gl_FragCoord.z;
}
