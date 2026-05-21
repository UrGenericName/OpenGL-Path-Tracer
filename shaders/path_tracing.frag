#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 faceNormal;
in vec3 intersectionPoint;

uniform vec3 sun;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D roughness;
uniform sampler2D metallic;

#define MAX_BRIGHTNESS 0.8
#define MIN_BRIGHTNESS 0.4

void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector);
void calculateWorldSpaceTangentBitagent(vec3 point, vec3 faceNormal, vec2 texCoord, out vec3 tangent, out vec3 bitangent);

void main()
{

	vec3 normalMapVector;
	vec4 temp = texture(normal, texCoord);
	normalMapVector.x = 2 * temp.x - 1;
	normalMapVector.y = 2 * temp.y - 1;
	normalMapVector.z = 2 * temp.z - 1;
	normalMapVector = normalize(normalMapVector);

	vec3 newFaceNormal;
	normalSpaceToWorldSpace(intersectionPoint, normalize(faceNormal), texCoord, normalMapVector, newFaceNormal);

	float tempDot = dot(normalize(newFaceNormal), normalize(sun));
	float brightness = tempDot * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;

	FragColor = texture(albedo, texCoord) * vec4(color, 1.0f) * brightness;
}

void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector) {

	vec3 tangent;
	vec3 bitangent;
	calculateWorldSpaceTangentBitagent(point, faceNormal, texCoord, tangent, bitangent);

	mat3 tangentToWorldSpaceMatrix = mat3(tangent, bitangent, faceNormal);

	outVector = normalize(tangentToWorldSpaceMatrix * inVector);

}

// finds world space tangent and bitangent; used in the normalToWorldSpace() function to find the transformation matrix
void calculateWorldSpaceTangentBitagent(vec3 point, vec3 faceNormal, vec2 texCoord, out vec3 tangent, out vec3 bitangent) {
			
	vec3 pointDx = dFdx(point);
	vec3 pointDy = dFdy(point);

	vec2 texCoordDx = dFdx(texCoord);
	vec2 texCoordDy = dFdy(texCoord);

	vec3 crossDx = cross(faceNormal, pointDx);
	vec3 crossDy = cross(pointDy, faceNormal);

	tangent = (crossDy * texCoordDx.x) + (crossDx * texCoordDy.x);
	bitangent = (crossDy * texCoordDx.y) + (crossDx * texCoordDy.y);

	float invmax = inversesqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));

	tangent = tangent * invmax;
	bitangent = bitangent * invmax;

}