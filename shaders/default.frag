#version 430 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 faceNormal;
in vec3 camOrientation;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D roughness;
uniform sampler2D metallic;

#define MAX_BRIGHTNESS 0.8
#define MIN_BRIGHTNESS 0.2

void main()
{
	float tempDot = dot(normalize(faceNormal), normalize(camOrientation));
	float brightness = abs(tempDot) * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;

	FragColor = texture(albedo, texCoord) * vec4(color, 1.0f) * brightness;
}