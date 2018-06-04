#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out V2F {
	vec3 posWorld;
	vec2 texCoords;
	vec3 tWorld;
	vec3 bWorld;
	vec3 nWorld;
	
	vec3 debugColor;
} v2f;


void main()
{
	v2f.debugColor = bitangent;
	
    gl_Position = projection * view * model * vec4(position, 1.0);
	
	v2f.posWorld = (model * vec4(position, 1.0)).xyz;
	v2f.texCoords = texCoords;
	
	mat3 normalMatrix = transpose(inverse(mat3(model)));

	vec3 tWorld = normalize(normalMatrix * tangent);
	vec3 bWorld = normalize(normalMatrix * bitangent);
	vec3 nWorld = normalize(normalMatrix * normal);
	
	v2f.tWorld = tWorld;
	v2f.bWorld = bWorld;
	v2f.nWorld = nWorld;

	return;
}
