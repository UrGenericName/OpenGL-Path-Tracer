#version 440 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 faceNormal;
in vec3 camOrientation;

layout(std430, binding = 2) readonly buffer MeshTextureBuffer {
    vec4 meshTextures[]; 
};

uniform uint currentMesh;
uniform uint albedo;
uniform uint normal;
uniform uint roughness;
uniform uint metallic;
uniform sampler2DArray texturePool;

#define MAX_BRIGHTNESS 0.8
#define MIN_BRIGHTNESS 0.2

void main()
{
	float tempDot = dot(normalize(faceNormal), normalize(camOrientation));
	float brightness = abs(tempDot) * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;

	FragColor = texture(texturePool, vec3(texCoord, albedo)) * vec4(color, 1.0f) * brightness;
}