#version 430 core

out vec4 FragColor;

layout(rgba32f, binding = 2) uniform image2D frameBuffer;

// INPUTS
in vec3 color;
in vec2 texCoord;
in vec3 geometricFaceNormal;
in vec3 intersectionPoint;
in vec3 rayOrientation;

uniform float u_debugMinBrightness;
uniform float u_debugMaxBrightness;
uniform bool u_debugHighlightObject;

uniform uint u_albedo;
uniform uint u_normal;
uniform uint u_roughness;
uniform uint u_metallic;
uniform float u_emissive;
uniform uint u_currentMesh;

const ivec2 pixelCoords = ivec2(gl_FragCoord.xy);

bool debugHighlightObject(inout vec4 inputColor);
void adjustBrightness(inout vec4 inputColor);

void main() {

	if (u_debugHighlightObject) {
		FragColor = vec4(1.0f);
		return;
	}

	if (u_emissive != 0.0f) {
		FragColor = vec4(color, 1.0f);
		return;
	}

	vec4 inputColor = imageLoad(frameBuffer, pixelCoords);

	adjustBrightness(inputColor);
	debugHighlightObject(inputColor);

	FragColor = inputColor;

}

bool debugHighlightObject(inout vec4 inputColor) {

	if (u_currentMesh == -1) {
		const vec4 highlightColor = vec4(1.0f, 0.6f, 0.0f, 1.0f);

		inputColor = mix(inputColor, highlightColor, 0.5f);
		inputColor *= ( (pixelCoords.x % 2) | (pixelCoords.y % 2) );
		return true;

	}

	return false;

}

void adjustBrightness(inout vec4 inputColor) {
	inputColor = inputColor * (u_debugMaxBrightness - u_debugMinBrightness) + u_debugMinBrightness;
}