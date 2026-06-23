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
uniform int u_debugHighlightedMesh;

uniform uint u_albedo;
uniform uint u_normal;
uniform uint u_roughness;
uniform uint u_metallic;
uniform float u_emissive;
uniform uint u_currentMesh;

uniform int u_isGizmo;
uniform int u_gizmoSelection;

const ivec2 pixelCoords = ivec2(gl_FragCoord.xy);

bool gizmoRender();
bool debugHighlightObject(inout vec4 inputColor);
void highlight(inout vec4 inputColor, in vec4 highlightColor, in float strength);
void adjustBrightness(inout vec4 inputColor);

void main() {

	if (gizmoRender()) return;

	vec4 inputColor;

	if (u_emissive != 0.0f) {
		inputColor = vec4(color, 1.0f);
		debugHighlightObject(inputColor);
		FragColor = inputColor;
		return;
	}

	inputColor = imageLoad(frameBuffer, pixelCoords);

	adjustBrightness(inputColor);
	debugHighlightObject(inputColor);

	FragColor = inputColor;

}

bool gizmoRender() {
	
	if (u_isGizmo != 0) {
		
		vec4 result = vec4(color, 1.0f);
		const vec4 highlightColor = vec4(1.0f, 0.8f, 0.4f, 1.0f);
		if (u_isGizmo == u_gizmoSelection) highlight(result, highlightColor, 0.8f);
		FragColor = result;
		return true;

	}

	return false;

}

bool debugHighlightObject(inout vec4 inputColor) {

	if (u_debugHighlightedMesh != -1 && u_currentMesh == u_debugHighlightedMesh) {
		const vec4 highlightColor = vec4(1.0f, 0.6f, 0.0f, 1.0f);
		highlight(inputColor, highlightColor, 0.5f);
		return true;

	}

	return false;

}

void highlight(inout vec4 inputColor, in vec4 highlightColor, in float strength) {

		inputColor = mix(inputColor, highlightColor, strength);
		inputColor *= ( (pixelCoords.x % 2) | (pixelCoords.y % 2) );

}

void adjustBrightness(inout vec4 inputColor) {
	inputColor = inputColor * (u_debugMaxBrightness - u_debugMinBrightness) + u_debugMinBrightness;
}