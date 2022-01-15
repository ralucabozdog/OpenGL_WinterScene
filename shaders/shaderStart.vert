#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fNormal;
out vec4 fPosEye;
out vec2 fTexCoords;
out vec4 fragPosLightSpace;
out vec3 lightPosEye;
out vec3 lightPosEye2;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform	mat3 normalMatrix;
uniform mat4 lightSpaceTrMatrix;
uniform vec3 pointLight;
uniform vec3 pointLight2;

void main() 
{
	//compute eye space coordinates
	fPosEye = view * model * vec4(vPosition, 1.0f);
	fNormal = normalize(normalMatrix * vNormal);
	fTexCoords = vTexCoords;
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	lightPosEye = (view * model * glm::vec4(pointLight, 1.0f)).xyz;
	lightPosEye2 = (view * model * glm::vec4(pointLight2, 1.0f)).xyz;

	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
}
