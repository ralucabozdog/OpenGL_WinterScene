#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec3 lightPosEye;
in vec3 lightPosEye2;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform float fogDensity;

uniform vec3 pointLight;
uniform vec3 pointLightColor;
uniform vec3 pointLight2;
uniform vec3 pointLightColor2;
uniform float incLinear;
uniform float incQuad;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.045f;
float quadratic = 0.0075f;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;

	// Check whether current frag pos is in shadow
	float bias = 0.005f;
	//float bias = max(0.05f * (1.0f - dot(normal, lightDir)), 0.005f);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	return shadow;
}

float computeFog()
{
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}

void computePointLight(){
	//transform normal
	vec3 normalEye = normalize(fNormal);

	//compute light direction
	vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(-fPosEye.xyz);

	vec3 reflection = normalize(reflect(lightDirN, normalEye));
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);

	//compute distance to light
	float dist = length(lightPosEye - fPosEye.xyz);
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	vec3 ambient1 = att * ambientStrength * pointLightColor * texture(diffuseTexture, fTexCoords).rgb;
	vec3 diffuse1 = att * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor * texture(diffuseTexture, fTexCoords).rgb;
	vec3 specular1 = att * specularStrength * specCoeff * pointLightColor * texture(specularTexture, fTexCoords).rgb;

	ambient += ambient1;
	diffuse += diffuse1;
	specular += specular1;
}

void computePointLight2(){
	//transform normal
	vec3 normalEye = normalize(fNormal);

	//compute light direction
	vec3 lightDirN = normalize(lightPosEye2 - fPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(-fPosEye.xyz);

	vec3 reflection = normalize(reflect(lightDirN, normalEye));
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);

	//compute distance to light
	float dist = length(lightPosEye2 - fPosEye.xyz);
	//compute attenuation
	float att = 1.0f / (constant + (linear + incLinear) * dist + (quadratic + incQuad) * (dist * dist));

	vec3 ambient1 = att * ambientStrength * pointLightColor2 * texture(diffuseTexture, fTexCoords).rgb;
	vec3 diffuse1 = att * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor2 * texture(diffuseTexture, fTexCoords).rgb;
	vec3 specular1 = att * specularStrength * specCoeff * pointLightColor2 * texture(specularTexture, fTexCoords).rgb;

	ambient += ambient1;
	diffuse += diffuse1;
	specular += specular1;
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	//vec3 color = min((ambient + diffuse) + specular, 1.0f);

	computePointLight();
	computePointLight2();
	
	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    
        float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	
	fColor = fogColor * (1 - fogFactor) + vec4(color, 1.0f) * fogFactor;
}
