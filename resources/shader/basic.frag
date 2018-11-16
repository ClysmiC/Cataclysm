#version 330

in V2F {
	vec3 posWorld;
	vec2 texCoords;
	vec3 tWorld;
	vec3 bWorld;
	vec3 nWorld;

	vec4 posLightSpace;
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

uniform sampler2D shadowMap;

out vec4 color;

float shadowValue()
{
    /* float bias = max(0.05 * (1 - dot(v2f.nWorld, directionalLights[0].direction)), 0.005); */
    float bias = 0; // this only works if we cull front faces. if we cull back faces, we need to add a bias to avoid shadow acne
    
	vec3 projectedCoords = v2f.posLightSpace.xyz; // / v2f.posLightSpace.w;
	projectedCoords = projectedCoords * 0.5 + 0.5; // [0, 1]

    if (projectedCoords.x < 0 || projectedCoords.x > 1) return 0;
    if (projectedCoords.y < 0 || projectedCoords.y > 1) return 0;
    if (projectedCoords.z > 1) return 0;
    
    float currentDepth = projectedCoords.z;

    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float shadow = 0;
    
    // use percentage-closer filtering to soften shadow resolution
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(shadowMap, projectedCoords.xy + vec2(x * texelSize.x, y * texelSize.y)).r;
            if (pcfDepth < currentDepth - bias) shadow += 1;
        }
    }

	return shadow / 9.0;;
}

vec3 directionalLight(DirectionalLight light, vec3 fragNormal, vec3 viewDir)
{
	vec3 toLightDir = -normalize(light.direction);
	float nDotL = max(dot(fragNormal, toLightDir), 0);

	vec3 halfwayDir = normalize(toLightDir + viewDir);
	float nDotH = max(dot(fragNormal, halfwayDir), 0);

	vec3 ambient = light.intensity * 0.1 * material.ambient * vec3(texture(material.ambientTex, v2f.texCoords));
    // ambient = vec3(0);
	vec3 diffuse = light.intensity * material.diffuse * vec3(texture(material.diffuseTex, v2f.texCoords)) * nDotL;
	vec3 specular = light.intensity *  material.specular * vec3(texture(material.specularTex, v2f.texCoords)) * pow(nDotH, material.specularExponent);

	// TODO: this shadow doesn't necessarily correspond to the light in this calculation,
	// so this can be wrong... for now I am sticking to only 1 directional light per scene,
	// so I am punting on this.
	
	return ambient + (1 - shadowValue()) * (diffuse + specular);
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

	vec3 ambient = light.intensity * 0.1 * material.ambient * vec3(texture(material.ambientTex, v2f.texCoords));
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
	
    vec3 result;

	for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
	{
		result += directionalLight(directionalLights[i], normal, viewDir);
	}
	
	for (int i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		result += pointLight(pointLights[i], normal, viewDir);
	}

	result = pow(result, vec3(1.0/2.2));
	color = vec4(result, 1.0);
}
