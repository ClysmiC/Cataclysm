#version 330

in V2F {
	vec3 posWorld;
	vec2 texCoords;
	vec3 tWorld;
	vec3 bWorld;
	vec3 nWorld;

	vec3 debugColor;
} v2f;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float specularExponent;

	sampler2D ambientTex;
	sampler2D diffuseTex;
	sampler2D specularTex;
	sampler2D normalTex;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 intensity;
};

struct PointLight
{
	vec3 posWorld;
	vec3 intensity;

	float attenuationConstant;
	float attenuationLinear;
	float attenuationQuadratic;
};

#define POINT_LIGHT_COUNT 1
uniform PointLight pointLights[POINT_LIGHT_COUNT];

// 0 : primary directional light
// 1 : secondary (back) directional light
#define DIRECTIONAL_LIGHT_COUNT 1
uniform DirectionalLight directionalLights[DIRECTIONAL_LIGHT_COUNT];

uniform Material material;
uniform vec3 cameraPosWorld;

out vec4 color;

vec3 directionalLight(DirectionalLight light, vec3 fragNormal, vec3 viewDir)
{
	return vec3(0, 0, 0);
	
	vec3 toLightDir = -normalize(light.direction);
	float nDotL = max(dot(fragNormal, toLightDir), 0);

	vec3 halfwayDir = normalize(toLightDir + viewDir);
	float nDotH = max(dot(fragNormal, halfwayDir), 0);

	vec3 ambient = light.intensity * material.ambient * vec3(texture(material.ambientTex, v2f.texCoords));
	vec3 diffuse = light.intensity * material.diffuse * vec3(texture(material.diffuseTex, v2f.texCoords)) * nDotL;
	vec3 specular = light.intensity *  material.specular * vec3(texture(material.specularTex, v2f.texCoords)) * pow(nDotH, material.specularExponent);

	return ambient + diffuse + specular;
}

vec3 pointLight(PointLight light, vec3 fragNormal, vec3 viewDir)
{
	vec3 toLightDir = normalize(light.posWorld - v2f.posWorld);
	float nDotL = max(dot(fragNormal, toLightDir), 0);

	vec3 halfwayDir = normalize(toLightDir + viewDir);
	float nDotH = max(dot(fragNormal, halfwayDir), 0);

	if (light.attenuationConstant < 1)
	{
		light.attenuationConstant = 1;
	}
	
	float lightDistance = length(light.posWorld - v2f.posWorld);
	float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * lightDistance + light.attenuationQuadratic * lightDistance * lightDistance);

	vec3 ambient = light.intensity * material.ambient * vec3(texture(material.ambientTex, v2f.texCoords));
	vec3 diffuse = light.intensity * material.diffuse * vec3(texture(material.diffuseTex, v2f.texCoords)) * nDotL;
	vec3 specular = light.intensity * material.specular * vec3(texture(material.specularTex, v2f.texCoords)) * pow(nDotH, material.specularExponent);

	return attenuation * (ambient + diffuse + specular);
}

void main()
{
	vec3 viewDir = normalize(cameraPosWorld - v2f.posWorld);

	vec3 normal = texture(material.normalTex, v2f.texCoords).rgb;
	normal = normalize(normal * 2 - 1);    // Convert xyz from [0, 1] to [-1, 1] before normalizing

	mat3 tbnWorld = mat3(v2f.tWorld, v2f.bWorld, v2f.nWorld);
	normal = tbnWorld * normal;

	/* normal = v2f.nWorld; */

	/* DEBUG VISUALIZE NORMALS */
	/* color = vec4((normal.x), (normal.y), (normal.z), 1.0); */
	/* return; */
	
    vec3 result;

	for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
	{
		// result += directionalLight(directionalLights[i], normal, viewDir);
	}
	
	for (int i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		result += pointLight(pointLights[i], normal, viewDir);
	}

	result = pow(result, vec3(1.0/2.2));
	color = vec4(result, 1.0);
}
